#include "dx12_shaders_compiler.hpp"

static std::string g_shaders_folder;

void dx12_shaders_compiler_set_folder(const char* const folder) {
    g_shaders_folder = folder;
}

std::unique_ptr<RHI_COMPILED_SHADER_BUFFER> dx12_shaders_compiler_compile(const char* const file,
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

    std::wstring file_name = to_wstring_ascii(file);
    std::wstring entry_point = to_wstring_ascii(entry);
    std::wstring target_lib = to_wstring_ascii(target);
    std::vector<LPCWSTR> args = {
        file_name.c_str(),
        L"-E", entry_point.c_str(),
        L"-T", target_lib.c_str(),
        L"-Zi",
        L"-Qembed_debug",
        L"-Od"
    };

    Microsoft::WRL::ComPtr<IDxcResult> result;
    HRESULT hr = compiler->Compile(
        &buffer,
        args.data(),
        (uint32_t)args.size(),
        nullptr,
        IID_PPV_ARGS(&result)
    );

    if (FAILED(hr)) {

        Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetStringLength() > 0)
        {
            OutputDebugStringA(errors->GetStringPointer());
        }
        throw std::exception("Shader compile failed");
    }

    result->GetStatus(&hr);
    if (FAILED(hr)) {

        Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetStringLength() > 0)
        {
            OutputDebugStringA(errors->GetStringPointer());
        }
        throw std::exception("Shader compile failed");
    }

    assert(SUCCEEDED(hr) && "Error compiling shader");

    IDxcBlob* shader;
    if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr))) {
        throw std::runtime_error("Failed to get compiled shader blob");
	}

	return std::make_unique<DX_COMPILED_SHADER_BUFFER>(shader);
}
