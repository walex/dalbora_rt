#include "rhi_hlsl_compiler.hpp"

#ifdef WINDOWS_PLATFORM
    #include <dxcapi.h>
    #define DXC_STRING_PREFIX(v) L##v
    using dxc_command_line_string = std::wstring;
    using dx_commnad_line_native_char = wchar_t;
    std::function<dxc_command_line_string(const std::string&)> to_command_line_string = to_wstring_ascii;
#else 
    #ifdef LINUX_PLATFORM
        #include <dxc/dxcapi.h>
        #define DXC_STRING_PREFIX(v) u##v
        using dxc_command_line_string = std::u16string;
        using dx_commnad_line_native_char = char16_t;
        std::function<dxc_command_line_string(const std::string&)> to_command_line_string = to_u16string;
    #else
        #error "Vulkan shader compiler not implemented for this platform"
    #endif
#endif


RHI_COMPILED_SHADER_BUFFER* rhi_hlsl_compiler_compile(const char* const file_path,
const char* const entry_point,
const char* const target_lib,
const char* const include_folder,
const byte_code_format format) {

    ASSERT_PTR(file_path);
    ASSERT_PTR(entry_point);
    ASSERT_PTR(target_lib);
    ASSERT_PTR(include_folder);

    // Declaración de los punteros nativos de las interfaces de DXC
    IDxcUtils* pUtils = nullptr;
    IDxcCompiler3* pCompiler = nullptr;
    IDxcResult* pResult = nullptr;
    IDxcBlobUtf8* pErrors = nullptr;
    IDxcBlob* pShaderObject = nullptr;
    IDxcIncludeHandler* pIncludeHandler = nullptr; // <--- AGREGA ESTO

    // 1. Obtener los puntos de entrada de las factorías de DXC
    DxcCreateInstanceProc pfnDxcCreateInstance = DxcCreateInstance;

    pfnDxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), (void**)&pUtils);
    pfnDxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler3), (void**)&pCompiler);
    pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

    // 2. Tu código fuente HLSL en memoria RAM
    std::string hlslSourceStr = load_string_from_file(file_path);

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = hlslSourceStr.c_str();
    sourceBuffer.Size = hlslSourceStr.size();
    sourceBuffer.Encoding = DXC_CP_UTF8;

    // 3. Argumentos en formato de 16 bits para Linux (char16_t)
    //std::vector<const char16_t*> arguments = {
    //    u"-E", u"main",
    //    u"-T", u"ps_6_0",
    //    u"-spirv",                    // Activar si el destino es Vulkan
    //    u"-fspv-target-env=vulkan1.2",
    //    u"-O3"
    //};

    dxc_command_line_string entry_point_str = to_command_line_string(entry_point);
    dxc_command_line_string target_lib_str = to_command_line_string(target_lib);
    dxc_command_line_string include_folder_str = to_command_line_string(include_folder);
    std::vector<const dx_commnad_line_native_char*> arguments = {
       DXC_STRING_PREFIX("-E"), entry_point_str.c_str(),
       DXC_STRING_PREFIX("-T"), target_lib_str.c_str(),
       DXC_STRING_PREFIX("-I"), include_folder_str.c_str()
#if defined(DEBUG)
        ,
        DXC_STRING_PREFIX("-Zi"),
        DXC_STRING_PREFIX("-Qembed_debug"),
        DXC_STRING_PREFIX("-Od")
#else
        ,
        DXC_STRING_PREFIX("-O3")
#endif
    };

    if (format & byte_code_format_output_spirv) {
        arguments.push_back(DXC_STRING_PREFIX("-spirv"));
        arguments.push_back(DXC_STRING_PREFIX("-fspv-target-env=vulkan1.3"));
    }

    // 4. Compilar en memoria
    HRESULT hr = pCompiler->Compile(
        &sourceBuffer,
        reinterpret_cast<LPCWSTR*>(arguments.data()),
        static_cast<UINT32>(arguments.size()),
        pIncludeHandler,
        __uuidof(IDxcResult),
        (void**)&pResult
    );

    if (FAILED(hr)) {
        
        LD("shader compiler", "invocation call failed");
        if (pIncludeHandler) pIncludeHandler->Release();
        if (pUtils) pUtils->Release();
        if (pCompiler) pCompiler->Release();
        throw std::exception("Shader compiler invocation call failed");
    }

    // 5. Verificar errores de compilación
    pResult->GetOutput(DXC_OUT_ERRORS, __uuidof(IDxcBlobUtf8), (void**)&pErrors, nullptr);
    if (pErrors && pErrors->GetStringLength() > 0) {
       
        HRESULT status;
        pResult->GetStatus(&status);
        if (FAILED(status)) {
            
            LD("shader compiler", pErrors->GetStringPointer());
            pErrors->Release();
            pResult->Release();
            if (pIncludeHandler) pIncludeHandler->Release();
            if (pUtils) pUtils->Release();
            if (pCompiler) pCompiler->Release();
            throw std::exception("Shader compilation failed");
        }
    }

    // 6. Extraer el código objeto (SPIR-V o DXIL binario) directamente de la RAM
    hr = pResult->GetOutput(DXC_OUT_OBJECT, __uuidof(IDxcBlob), (void**)&pShaderObject, nullptr);

    //if (FAILED(hr)) {
    //
    //    // Punteros listos para consumir en memoria RAM
    //    const void* pBytecode = pShaderObject->GetBufferPointer();
    //    size_t bytecodeSize = pShaderObject->GetBufferSize();

    //    std::cout << "¡Éxito! Código objeto en memoria: " << bytecodeSize << " bytes." << std::endl;

    //    // Aquí usarías el binario (ej. vkCreateShaderModule o guardarlo en tu propia caché de RAM)
    //}

    if (pErrors)       pErrors->Release();
    if (pResult)       pResult->Release();
    if (pCompiler)     pCompiler->Release();
    if (pIncludeHandler) pIncludeHandler->Release();
    if (pUtils)        pUtils->Release();
 
    RHI_COMPILED_SHADER_BUFFER* result = new RHI_COMPILED_SHADER_BUFFER();
    ASSERT_PTR(result);
    result->buffer_size = pShaderObject->GetBufferSize();
	if (pShaderObject->GetBufferPointer() && result->buffer_size > 0) {
        result->buffer = new uint8_t[pShaderObject->GetBufferSize()];
        ASSERT_PTR(result->buffer);
		memcpy(result->buffer, pShaderObject->GetBufferPointer(), result->buffer_size);
	}
    pShaderObject->Release();
    return result;
}

void rhi_hlsl_compiler_compile_to_file(const char* const file_path,
    const char* const entry_point,
    const char* const target_lib, 
    const char* const include_folder,
    const byte_code_format format,
    const char* const out_file_path) {

	RHI_COMPILED_SHADER_BUFFER* compiled_shader = rhi_hlsl_compiler_compile(file_path, entry_point, target_lib, include_folder, format);
	ASSERT_PTR(compiled_shader);
	ASSERT_EXPR(compiled_shader->buffer_size > 0);
	ASSERT_PTR(compiled_shader->buffer);
    save_binary_to_file(out_file_path, compiled_shader->buffer, compiled_shader->buffer_size);
	delete compiled_shader;
}

//RHI_COMPILED_SHADER_BUFFER* dx12_shaders_compiler_compile_hlsl(const char* const file_path, const char* const include_folder, 
//    const char* const entry, const char* const target, 
//    byte_code_format format) {
//
//    ASSERT_PTR(file_path);
//    ASSERT_PTR(include_folder);
//    ASSERT_PTR(entry);
//    ASSERT_PTR(target);


//    Microsoft::WRL::ComPtr<IDxcUtils> utils;
//    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
//    ASSERT_COM_SUCCESS(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
//    ASSERT_COM_SUCCESS(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
//
//    auto sourceData = file_system_file_to_bin(file_path);
//
//    DxcBuffer buffer = {};
//    buffer.Ptr = sourceData.data();
//    buffer.Size = sourceData.size();
//    buffer.Encoding = DXC_CP_UTF8;
//
//    std::wstring file_name = to_wstring_ascii(file_path);
//    std::wstring entry_point = to_wstring_ascii(entry);
//    std::wstring target_lib = to_wstring_ascii(target);
//    std::wstring w_include_folder = to_wstring_ascii(include_folder);
//    std::vector<LPCWSTR> args = {
//        file_name.c_str(),
//        L"-E", entry_point.c_str(),
//        L"-T", target_lib.c_str(),
//        L"-I", w_include_folder.c_str()
//#if defined(DEBUG)
//        ,  
//        L"-Zi",
//        L"-Qembed_debug",
//        L"-Od"
//#else
//		,
//		L"-O3"
//#endif
//    };
//    if (format & byte_code_format_spirv_output) {
//        args.push_back(L"-spirv");
//        args.push_back(L"-fspv-target-env=vulkan1.3");
//    }
//    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;
//    ASSERT_COM_SUCCESS(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));
//
//    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;
//    ASSERT_COM_SUCCESS(dxcUtils->CreateDefaultIncludeHandler(&includeHandler));
//    
//    Microsoft::WRL::ComPtr<IDxcResult> i_shader_compiled;
//    HRESULT hr = compiler->Compile(
//        &buffer,
//        args.data(),
//        (uint32_t)args.size(),
//        includeHandler.Get(),
//        IID_PPV_ARGS(&i_shader_compiled)
//    );
//
//    if (FAILED(hr)) {
//
//        Microsoft::WRL::ComPtr<IDxcBlobUtf8> i_errors;
//        i_shader_compiled->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&i_errors), nullptr);
//        if (i_errors && i_errors->GetStringLength() > 0)
//        {
//            OutputDebugStringA(i_errors->GetStringPointer());
//        }
//        throw std::exception("Shader compile failed");
//    }
//
//    i_shader_compiled->GetStatus(&hr);
//    if (FAILED(hr)) {
//
//        Microsoft::WRL::ComPtr<IDxcBlobUtf8> i_errors;
//        i_shader_compiled->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&i_errors), nullptr);
//        if (i_errors && i_errors->GetStringLength() > 0)
//        {
//            OutputDebugStringA(i_errors->GetStringPointer());
//        }
//        throw std::exception("Shader compile failed");
//    }
//
//    IDxcBlob* i_shader;
//    if (FAILED(i_shader_compiled->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&i_shader), nullptr))) {
//        throw std::runtime_error("Failed to get compiled shader blob");
//	}
//    ASSERT_EXPR(i_shader->GetBufferSize() > 0);
//    ASSERT_PTR(i_shader->GetBufferPointer());
//
//    DX_COMPILED_SHADER_BUFFER* result = new DX_COMPILED_SHADER_BUFFER();
//    ASSERT_PTR(result);
//    result->set_handle(i_shader);
//    return result;
//}



//size_t dx12_shaders_compiler_get_bytecode_ptr(const RHI_COMPILED_SHADER_BUFFER* const shader,
//    const char* const file_path, uint8_t** bytecode) {
//
//    ASSERT_PTR(bytecode);
//
//    if (shader == nullptr) {
//        *bytecode = nullptr;
//        return 0;
//    }
//
//    DX_COMPILED_SHADER_BUFFER* dx_shader = static_cast<DX_COMPILED_SHADER_BUFFER*>(const_cast<RHI_COMPILED_SHADER_BUFFER*>(shader));
//    IDxcBlob* i_shader = static_cast<IDxcBlob*>(dx_shader->get_handle());
//    ASSERT_PTR(i_shader);
//    *bytecode = static_cast<uint8_t*>(i_shader->GetBufferPointer());
//    return i_shader->GetBufferSize();
//}

//RHI_COMPILED_SHADER_BUFFER* dx12_shaders_compiler_set_bytecode_ptr(RHI_COMPILED_SHADER_BUFFER* const shader, const uint8_t* const bytecode,
//	const size_t size) {
//	ASSERT_PTR(bytecode);
//	ASSERT_EXPR(size > 0);
//
//    Microsoft::WRL::ComPtr<IDxcUtils> dxc_utils;
//    ASSERT_COM_SUCCESS(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils)));
//
//    Microsoft::WRL::ComPtr<IDxcBlobEncoding> i_blob_enc;
//	HRESULT hr = dxc_utils->CreateBlobFromPinned(bytecode, size, CP_UTF8, &i_blob_enc);
//
//    if (FAILED(hr))
//    {
//        throw std::runtime_error("Error al creating Blob from pinned memory");
//    }
//
//    Microsoft::WRL::ComPtr<IDxcBlob> i_blob;
//    ASSERT_COM_SUCCESS(i_blob_enc.As(&i_blob));
//
//    DX_COMPILED_SHADER_BUFFER* result = new DX_COMPILED_SHADER_BUFFER();
//    ASSERT_PTR(result);
//    if (shader) {
//        IDxcBlob* i_old_shader = *static_cast<DX_COMPILED_SHADER_BUFFER*>(shader);
//        if (i_old_shader) {
//            i_old_shader->Release();
//        }
//        delete shader;
//    }
//    result->set_handle(i_blob.Get());
//    return result;
//}