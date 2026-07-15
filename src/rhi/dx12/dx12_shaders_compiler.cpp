#include "dx12_shaders_compiler.hpp"

static std::string g_shaders_folder;

void dx12_shaders_compiler_set_folder(const char* const folder) {
    if (std::filesystem::exists(folder) == false)
        throw "shader path not found";
    g_shaders_folder = folder;
}

RHI_COMPILED_SHADER_BUFFER* dx12_shaders_compiler_compile(const char* const file,
	const char* const entry, 
	const char* const target) {

    ASSERT_PTR(file);
    ASSERT_PTR(entry);
    ASSERT_PTR(target);

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
    std::wstring include_folder = to_wstring_ascii(g_shaders_folder);
    std::vector<LPCWSTR> args = {
        file_name.c_str(),
        L"-E", entry_point.c_str(),
        L"-T", target_lib.c_str(),
        L"-I", include_folder.c_str()
#if defined(DEBUG)
        ,  
        L"-Zi",
        L"-Qembed_debug",
        L"-Od"
#endif
    };

    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
    HRESULT hr = DxcCreateInstance(
        CLSID_DxcUtils,
        IID_PPV_ARGS(&dxcUtils));

    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
    dxcUtils->CreateDefaultIncludeHandler(
        &includeHandler);
    
    Microsoft::WRL::ComPtr<IDxcResult> i_shader_compiled;
    hr = compiler->Compile(
        &buffer,
        args.data(),
        (uint32_t)args.size(),
        includeHandler.Get(),
        IID_PPV_ARGS(&i_shader_compiled)
    );

    if (FAILED(hr)) {

        Microsoft::WRL::ComPtr<IDxcBlobUtf8> i_errors;
        i_shader_compiled->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&i_errors), nullptr);
        if (i_errors && i_errors->GetStringLength() > 0)
        {
            OutputDebugStringA(i_errors->GetStringPointer());
        }
        throw std::exception("Shader compile failed");
    }

    i_shader_compiled->GetStatus(&hr);
    if (FAILED(hr)) {

        Microsoft::WRL::ComPtr<IDxcBlobUtf8> i_errors;
        i_shader_compiled->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&i_errors), nullptr);
        if (i_errors && i_errors->GetStringLength() > 0)
        {
            OutputDebugStringA(i_errors->GetStringPointer());
        }
        throw std::exception("Shader compile failed");
    }

    IDxcBlob* i_shader;
    if (FAILED(i_shader_compiled->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&i_shader), nullptr))) {
        throw std::runtime_error("Failed to get compiled shader blob");
	}
    ASSERT_EXPR(i_shader->GetBufferSize() > 0);
    ASSERT_PTR(i_shader->GetBufferPointer());

    DX_COMPILED_SHADER_BUFFER* result = new DX_COMPILED_SHADER_BUFFER();
    ASSERT_PTR(result);
    result->set_handle(i_shader);
    return result;
}
