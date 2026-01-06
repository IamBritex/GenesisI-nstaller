#pragma once
#include <windows.h>
#include <wrl.h>
#include <string>
#include <filesystem>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include "Config.h"
#include "Utils.h"
#include "NativeBridge.h"
#include "GenesisAPI.h" // <--- IMPORTAMOS LA API JS

using namespace Microsoft::WRL;
namespace fs = std::filesystem;

class WebViewManager {
public:
    static bool isLoaded;

    static void Initialize(HWND hWnd, std::wstring appID, AppConfig config) {
        isLoaded = false;
        auto options = Make<CoreWebView2EnvironmentOptions>();
        options->put_AdditionalBrowserArguments(L"--disable-gpu --no-proxy-server --disable-features=Translate,OptimizationHints,MediaRouter");

        std::wstring userDataFolder = Utils::GetAppDataPath(appID) + L"\\profile";

        CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), options.Get(),
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [hWnd, appID, config](HRESULT res, ICoreWebView2Environment* env) -> HRESULT {
                    
                    if (FAILED(res) || !env) { 
                        MessageBoxW(hWnd, L"Error Critico: WebView2", L"Error", MB_OK);
                        return S_FALSE; 
                    }
                    
                    env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hWnd, appID, config](HRESULT, ICoreWebView2Controller* c) -> HRESULT {
                            if (!c) return S_FALSE; 
                            c->AddRef();
                            SendMessage(hWnd, WM_USER + 1, 0, (LPARAM)c);
                            RECT b; GetClientRect(hWnd, &b); c->put_Bounds(b);
                            ComPtr<ICoreWebView2> wv; c->get_CoreWebView2(&wv);
                            ComPtr<ICoreWebView2_3> wv3; wv.As(&wv3);

                            ComPtr<ICoreWebView2Settings> settings; wv->get_Settings(&settings);
                            settings->put_IsScriptEnabled(TRUE); 
                            settings->put_AreDevToolsEnabled(config.devTools ? TRUE : FALSE);
                            settings->put_AreDefaultContextMenusEnabled(FALSE);
                            
                            // Fondo transparente para efectos visuales
                            COREWEBVIEW2_COLOR color = { 0, 0, 0, 0 };
                            ComPtr<ICoreWebView2Controller2> c2; c->QueryInterface(IID_PPV_ARGS(&c2));
                            if (c2) c2->put_DefaultBackgroundColor(color);

                            // Mapeo de carpetas AppData
                            std::wstring assetsPath = Utils::GetAppDataPath(appID) + L"\\assets";
                            std::wstring domain = appID.empty() ? L"app.genesis" : appID;
                            if (wv3) wv3->SetVirtualHostNameToFolderMapping(domain.c_str(), assetsPath.c_str(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);

                            // --- INYECCIÓN AUTOMÁTICA DE LA API ---
                            // Esto inyecta el JS antes de que cargue la pagina
                            wv->AddScriptToExecuteOnDocumentCreated(GENESIS_JS_API.c_str(), nullptr);

                            // Evento de carga completada (para quitar pantalla de carga negra)
                            wv->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                [hWnd](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                    isLoaded = true;
                                    InvalidateRect(hWnd, NULL, TRUE);
                                    return S_OK;
                                }).Get(), nullptr);
                            
                            // Puente de Comunicación (Recibe mensajes del JS inyectado)
                            wv->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                [hWnd](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                    LPWSTR p; args->TryGetWebMessageAsString(&p);
                                    std::wstring msg(p); CoTaskMemFree(p);
                                    
                                    // Procesar con el puente nativo
                                    NativeBridge::HandleApiCall(hWnd, msg, sender);
                                    
                                    // Manejo legacy para funciones viejas si las hubiera
                                    if (msg.find(L"openExternal:") == 0) ShellExecuteW(NULL, L"open", msg.substr(13).c_str(), NULL, NULL, SW_SHOWNORMAL);
                                    
                                    return S_OK;
                                }).Get(), nullptr);

                            std::wstring url = L"https://" + domain + L"/" + config.entryPoint;
                            wv->Navigate(url.c_str());
                            return S_OK;
                        }).Get());
                    return S_OK;
                }).Get());
    }
};

bool WebViewManager::isLoaded = false;