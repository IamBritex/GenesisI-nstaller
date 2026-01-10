#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <wrl.h>
#include <string>
#include <algorithm>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"
#include "fileSystem.h"

using namespace Microsoft::WRL;

class WebViewInstance
{
public:
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;
    EventRegistrationToken token;

    void Initialize(HWND hWnd, const WindowConfig &config)
    {
        // Usamos una carpeta nueva "v9_root_fix" para reiniciar la caché de rutas
        std::string path = FileSystem::getAppDataPath() + "\\webview_data_v9_root_fix";
        std::wstring userDataFolder(path.begin(), path.end());

        auto options = Make<CoreWebView2EnvironmentOptions>();

        if (options)
        {
            std::wstring flags = L"";

            // --- RED Y SEGURIDAD ---
            flags += L"--disable-web-security ";
            flags += L"--disable-site-isolation-trials ";
            flags += L"--allow-running-insecure-content ";
            flags += L"--allow-insecure-localhost ";
            flags += L"--no-proxy-server ";

            // --- GPU (Estabilidad D3D11) ---
            flags += L"--use-angle=d3d11 ";
            flags += L"--enable-unsafe-swiftshader ";
            flags += L"--ignore-gpu-blocklist ";
            flags += L"--enable-webgl ";

            // --- UI ---
            flags += L"--disable-features=msSmartScreenProtection,TrackingPrevention,msSmartScreen ";
            flags += L"--disable-blink-features=AutomationControlled ";

            options->put_AdditionalBrowserArguments(flags.c_str());

            ComPtr<ICoreWebView2EnvironmentOptions5> options5;
            if (options.As(&options5) == S_OK)
                options5->put_EnableTrackingPrevention(FALSE);
        }

        CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataFolder.c_str(), options.Get(),
                                                 Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                                                     [hWnd, config, this](HRESULT res, ICoreWebView2Environment *env) -> HRESULT
                                                     {
                                                         if (!env)
                                                         {
                                                             MessageBox(hWnd, L"Error Crítico WebView2", L"Error", MB_OK);
                                                             return S_FALSE;
                                                         }

                                                         env->CreateCoreWebView2Controller(hWnd,
                                                                                           Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                                                                               [hWnd, config, this](HRESULT res, ICoreWebView2Controller *host) -> HRESULT
                                                                                               {
                                                                                                   if (!host)
                                                                                                       return S_FALSE;
                                                                                                   this->controller = host;
                                                                                                   this->controller->get_CoreWebView2(&this->webview);

                                                                                                   ComPtr<ICoreWebView2Settings> settings;
                                                                                                   this->webview->get_Settings(&settings);
                                                                                                   if (settings)
                                                                                                   {
                                                                                                       settings->put_IsScriptEnabled(TRUE);
                                                                                                       settings->put_AreDevToolsEnabled(TRUE);
                                                                                                       settings->put_IsWebMessageEnabled(TRUE);
                                                                                                       settings->put_AreDefaultContextMenusEnabled(FALSE);
                                                                                                       settings->put_IsStatusBarEnabled(FALSE);
                                                                                                   }

                                                                                                   // Fondo transparente
                                                                                                   COREWEBVIEW2_COLOR transparentColor = {0, 0, 0, 0};
                                                                                                   ComPtr<ICoreWebView2Controller2> controller2;
                                                                                                   if (this->controller.As(&controller2) == S_OK)
                                                                                                   {
                                                                                                       controller2->put_DefaultBackgroundColor(transparentColor);
                                                                                                   }

                                                                                                   // Tracking Prevention OFF
                                                                                                   ComPtr<ICoreWebView2_13> webview13;
                                                                                                   if (this->webview.As(&webview13) == S_OK)
                                                                                                   {
                                                                                                       ComPtr<ICoreWebView2Profile> profile;
                                                                                                       webview13->get_Profile(&profile);
                                                                                                       if (profile)
                                                                                                       {
                                                                                                           ComPtr<ICoreWebView2Profile3> profile3;
                                                                                                           if (profile.As(&profile3) == S_OK)
                                                                                                               profile3->put_PreferredTrackingPreventionLevel(COREWEBVIEW2_TRACKING_PREVENTION_LEVEL_NONE);
                                                                                                       }
                                                                                                   }

                                                                                                   // --- [CORRECCIÓN CRÍTICA DE RUTAS] ---
                                                                                                   ComPtr<ICoreWebView2_3> webview3;
                                                                                                   if (this->webview.As(&webview3) == S_OK)
                                                                                                   {
                                                                                                       // ANTES: Mapeábamos solo ".../Genesis/assets" -> installer.genesis/
                                                                                                       // AHORA: Mapeamos ".../Genesis" (la raíz) -> installer.genesis/
                                                                                                       // Esto permite que los scripts accedan a "assets" Y "core" (usando ../core)

                                                                                                       std::string rootPath = FileSystem::getAppDataPath(); // La carpeta raíz de la app
                                                                                                       std::wstring wRootPath(rootPath.begin(), rootPath.end());

                                                                                                       webview3->SetVirtualHostNameToFolderMapping(
                                                                                                           L"installer.genesis", wRootPath.c_str(),
                                                                                                           COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
                                                                                                   }

                                                                                                   // --- INTERCEPTOR DE HEADERS (MIME TYPE & CORS) ---
                                                                                                   ComPtr<ICoreWebView2_2> webview2_2;
                                                                                                   if (this->webview.As(&webview2_2) == S_OK)
                                                                                                   {
                                                                                                       webview2_2->add_WebResourceResponseReceived(
                                                                                                           Callback<ICoreWebView2WebResourceResponseReceivedEventHandler>(
                                                                                                               [](ICoreWebView2 *sender, ICoreWebView2WebResourceResponseReceivedEventArgs *args) -> HRESULT
                                                                                                               {
                                                                                                                   ComPtr<ICoreWebView2WebResourceRequest> request;
                                                                                                                   args->get_Request(&request);
                                                                                                                   LPWSTR uri;
                                                                                                                   request->get_Uri(&uri);
                                                                                                                   std::wstring wUri(uri);
                                                                                                                   CoTaskMemFree(uri);
                                                                                                                   std::transform(wUri.begin(), wUri.end(), wUri.begin(), ::tolower);

                                                                                                                   ComPtr<ICoreWebView2WebResourceResponseView> view;
                                                                                                                   args->get_Response(&view);
                                                                                                                   if (view)
                                                                                                                   {
                                                                                                                       ComPtr<ICoreWebView2HttpResponseHeaders> headers;
                                                                                                                       view->get_Headers(&headers);
                                                                                                                       if (headers)
                                                                                                                       {
                                                                                                                           headers->AppendHeader(L"Access-Control-Allow-Origin", L"*");
                                                                                                                           if (wUri.find(L".js") != std::wstring::npos)
                                                                                                                           {
                                                                                                                               headers->AppendHeader(L"Content-Type", L"application/javascript");
                                                                                                                           }
                                                                                                                           else if (wUri.find(L".json") != std::wstring::npos)
                                                                                                                           {
                                                                                                                               headers->AppendHeader(L"Content-Type", L"application/json");
                                                                                                                           }
                                                                                                                       }
                                                                                                                   }
                                                                                                                   return S_OK;
                                                                                                               })
                                                                                                               .Get(),
                                                                                                           &token);
                                                                                                   }

                                                                                                   RECT bounds;
                                                                                                   GetClientRect(hWnd, &bounds);
                                                                                                   this->controller->put_Bounds(bounds);
                                                                                                   this->controller->put_IsVisible(TRUE);

                                                                                                   // --- NAVEGACIÓN CORREGIDA ---
                                                                                                   // Como ahora la raíz es "Genesis", debemos entrar explícitamente a "assets/"
                                                                                                   std::string entry = "https://installer.genesis/assets/" + config.build.entryPoint;
                                                                                                   std::wstring wEntry(entry.begin(), entry.end());

                                                                                                   this->webview->Navigate(wEntry.c_str());
                                                                                                   this->webview->OpenDevToolsWindow();

                                                                                                   return S_OK;
                                                                                               })
                                                                                               .Get());
                                                         return S_OK;
                                                     })
                                                     .Get());
    }

    void Resize(HWND hWnd)
    {
        if (controller)
        {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            controller->put_Bounds(bounds);
        }
    }
};

#endif