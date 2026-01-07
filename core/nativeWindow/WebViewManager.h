#pragma once
#include <windows.h>
#include <shellapi.h>
#include <wrl.h>
#include <string>
#include <filesystem>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include "Config.h"
#include "Utils.h"
#include "NativeBridge.h"
#include "GenesisAPI.h"
#include "LocalServer.h"

using namespace Microsoft::WRL;
namespace fs = std::filesystem;

class WebViewManager
{
public:
    static bool isLoaded;

    static void Initialize(HWND hWnd, std::wstring appID, AppConfig config)
    {
        isLoaded = false;
        auto options = Make<CoreWebView2EnvironmentOptions>();
        // Mantenemos los flags agresivos por si acaso
        options->put_AdditionalBrowserArguments(L"--disable-features=Translate,OptimizationHints,MediaRouter,TrackingPrevention,SmartScreen --autoplay-policy=no-user-gesture-required --allow-file-access-from-files");

        std::wstring userDataFolder = Utils::GetAppDataPath(appID) + L"\\profile";

        CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), options.Get(),
                                                 Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                                                     [hWnd, appID, config](HRESULT res, ICoreWebView2Environment *env) -> HRESULT
                                                     {
                                                         if (FAILED(res) || !env)
                                                         {
                                                             MessageBoxW(hWnd, L"Error Critico: WebView2", L"Error", MB_OK);
                                                             return S_FALSE;
                                                         }

                                                         env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                                                                                     [hWnd, appID, config](HRESULT, ICoreWebView2Controller *c) -> HRESULT
                                                                                                     {
                                                                                                         if (!c)
                                                                                                             return S_FALSE;
                                                                                                         c->AddRef();
                                                                                                         SendMessage(hWnd, WM_USER + 1, 0, (LPARAM)c);

                                                                                                         RECT b;
                                                                                                         GetClientRect(hWnd, &b);
                                                                                                         c->put_Bounds(b);

                                                                                                         ComPtr<ICoreWebView2> wv;
                                                                                                         c->get_CoreWebView2(&wv);

                                                                                                         // ---------------------------------------------------------
                                                                                                         // CORRECCIÓN: Usar ICoreWebView2Profile3 para TrackingPrevention
                                                                                                         // ---------------------------------------------------------
                                                                                                         ComPtr<ICoreWebView2_13> wv13;
                                                                                                         wv.As(&wv13);
                                                                                                         if (wv13)
                                                                                                         {
                                                                                                             ComPtr<ICoreWebView2Profile> profile;
                                                                                                             wv13->get_Profile(&profile);
                                                                                                             if (profile)
                                                                                                             {
                                                                                                                 // Casteamos a Profile3 que es quien tiene el metodo
                                                                                                                 ComPtr<ICoreWebView2Profile3> profile3;
                                                                                                                 profile.As(&profile3);
                                                                                                                 if (profile3)
                                                                                                                 {
                                                                                                                     profile3->put_PreferredTrackingPreventionLevel(COREWEBVIEW2_TRACKING_PREVENTION_LEVEL_NONE);
                                                                                                                 }
                                                                                                             }
                                                                                                         }
                                                                                                         // ---------------------------------------------------------

                                                                                                         ComPtr<ICoreWebView2Settings> settings;
                                                                                                         wv->get_Settings(&settings);

                                                                                                         settings->put_IsScriptEnabled(TRUE);
                                                                                                         settings->put_AreDevToolsEnabled(config.devTools ? TRUE : FALSE);
                                                                                                         settings->put_AreDefaultContextMenusEnabled(FALSE);
                                                                                                         settings->put_IsZoomControlEnabled(FALSE);

                                                                                                         COREWEBVIEW2_COLOR color = {0, 0, 0, 0};
                                                                                                         ComPtr<ICoreWebView2Controller2> c2;
                                                                                                         c->QueryInterface(IID_PPV_ARGS(&c2));
                                                                                                         if (c2)
                                                                                                             c2->put_DefaultBackgroundColor(color);

                                                                                                         wv->AddScriptToExecuteOnDocumentCreated(GENESIS_JS_API.c_str(), nullptr);

                                                                                                         wv->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                                                                                                                         [hWnd](ICoreWebView2 *, ICoreWebView2NavigationCompletedEventArgs *args) -> HRESULT
                                                                                                                                         {
                                                                                                                                             isLoaded = true;
                                                                                                                                             InvalidateRect(hWnd, NULL, TRUE);
                                                                                                                                             return S_OK;
                                                                                                                                         })
                                                                                                                                         .Get(),
                                                                                                                                     nullptr);

                                                                                                         wv->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                                                                                                                        [hWnd](ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) -> HRESULT
                                                                                                                                        {
                                                                                                                                            LPWSTR p;
                                                                                                                                            args->TryGetWebMessageAsString(&p);
                                                                                                                                            std::wstring msg(p);
                                                                                                                                            CoTaskMemFree(p);

                                                                                                                                            NativeBridge::HandleApiCall(hWnd, msg, sender);

                                                                                                                                            if (msg.find(L"openExternal:") == 0)
                                                                                                                                                ShellExecuteW(NULL, L"open", msg.substr(13).c_str(), NULL, NULL, SW_SHOWNORMAL);

                                                                                                                                            return S_OK;
                                                                                                                                        })
                                                                                                                                        .Get(),
                                                                                                                                    nullptr);

                                                                                                         std::wstring url = L"http://localhost:" + std::to_wstring(LocalServer::port) + L"/" + config.entryPoint;
                                                                                                         wv->Navigate(url.c_str());
                                                                                                         return S_OK;
                                                                                                     })
                                                                                                     .Get());
                                                         return S_OK;
                                                     })
                                                     .Get());
    }
};

bool WebViewManager::isLoaded = false;