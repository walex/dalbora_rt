#include "dx12_fence.hpp"
#include "strings.hpp"
#include "file_system.hpp"

static std::string g_shaders_folder;

void dx12_shaders_set_folder(const std::string& folder) {
    g_shaders_folder = folder;
}

const std::string& dx12_shaders_get_folder() {
    return g_shaders_folder;
}

std::unique_ptr<RHI_OBJECT> dx12_shaders_compile(const char* const file,
	const char* const entry, 
	const char* const target) {

    Microsoft::WRL::ComPtr<IDxcUtils> utils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	std::string fullPath = file_system_path_join(g_shaders_folder, file).string();
    auto sourceData = file_system_file_to_bin(fullPath.c_str());

    DxcBuffer buffer = {};
    buffer.Ptr = sourceData.data();
    buffer.Size = sourceData.size();
    buffer.Encoding = DXC_CP_UTF8;

    std::vector<LPCWSTR> args = {
        utf8_to_wstring(file).c_str(),
        L"-E", utf8_to_wstring(entry).c_str(),
        L"-T", utf8_to_wstring(target).c_str(),
        L"-Zi",
        L"-Qembed_debug",
        L"-Od"
    };

    Microsoft::WRL::ComPtr<IDxcResult> result;
    compiler->Compile(
        &buffer,
        args.data(),
        (uint32_t)args.size(),
        nullptr,
        IID_PPV_ARGS(&result)
    );

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetStringLength() > 0)
    {
        OutputDebugStringA(errors->GetStringPointer());
    }

    HRESULT hr;
    result->GetStatus(&hr);
    assert(SUCCEEDED(hr) && "Error compilando shader");

    IDxcBlob* shader;
    if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr))) {
        throw std::runtime_error("Failed to get compiled shader blob");
	}

	return std::make_unique<RHI_OBJECT>(new DX_SHADER_HANDLE(shader));
}
