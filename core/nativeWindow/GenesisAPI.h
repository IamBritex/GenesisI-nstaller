#pragma once
#include <string>

const std::wstring GENESIS_JS_API = LR"(
(function() {
    if (window.genesis) return;

    const isDesktop = !!(window.chrome && window.chrome.webview);
    const envType = isDesktop ? 'desktop' : 'web';

    const _post = function(payload) {
        if (isDesktop) {
            window.chrome.webview.postMessage(JSON.stringify(payload));
        } else {
            console.log(`%c[Genesis Mock] ${payload.group}:${payload.action}`, 'color: cyan', payload);
        }
    };

    window.genesis = {
        env: envType,
        isNative: isDesktop,

        window: {
            close: () => _post({ group: "window", action: "close" }),
            minimize: () => _post({ group: "window", action: "minimize" }),
            maximize: () => _post({ group: "window", action: "maximize" }),
            restore: () => _post({ group: "window", action: "restore" }),
            flash: () => _post({ group: "window", action: "flash" }),
            shake: () => _post({ group: "window", action: "shake" }),
            drag: () => _post({ group: "window", action: "drag" }),
            center: () => _post({ group: "window", action: "center" }),
            showTitleBar: (s) => _post({ group: "window", action: "showTitleBar", param: s }),
            setTitle: (t) => _post({ group: "window", action: "setTitle", param: t }),
            setIcon: (p) => _post({ group: "window", action: "setIcon", param: p }),
            move: (x, y) => _post({ group: "window", action: "move", x: x, y: y }),
            moveBy: (dx, dy) => _post({ group: "window", action: "moveBy", x: dx, y: dy }),
            resize: (w, h) => _post({ group: "window", action: "resize", w: w, h: h }),
            dock: (p) => _post({ group: "window", action: "dock", param: p }),
            fullscreen: (a) => _post({ group: "window", action: "fullscreen", param: a }),
            setTransparent: (a) => _post({ group: "window", action: "setTransparent", param: a }),
            setOpacity: (a) => _post({ group: "window", action: "setOpacity", param: a }),
            topmost: (a) => _post({ group: "window", action: "topmost", param: a }),
            getSize: () => _post({ group: "window", action: "getSize" })
        },

        system: {
            setWallpaper: (p) => _post({ group: "system", action: "wallpaper", param: p }),
            isAdmin: () => _post({ group: "system", action: "isAdmin" }),
            getScreenSize: () => _post({ group: "system", action: "getScreenSize" }),
            getMemory: () => _post({ group: "system", action: "getMemoryInfo" })
        },

        fs: {
            write: (p, d) => _post({ group: "fs", action: "write", path: p, data: d }),
            read: (p) => _post({ group: "fs", action: "read", path: p }),
            delete: (p) => _post({ group: "fs", action: "delete", path: p }),
            mkdir: (p) => _post({ group: "fs", action: "mkdir", path: p }),
            rename: (o, n) => _post({ group: "fs", action: "rename", oldPath: o, newPath: n }),
            exists: (p) => _post({ group: "fs", action: "exists", path: p }),
            list: (p) => _post({ group: "fs", action: "list", path: p })
        },

        utils: {
            clipboardWrite: (t) => _post({ group: "utils", action: "clipboardWrite", param: t }),
            clipboardRead: () => _post({ group: "utils", action: "clipboardRead" }),
            notify: (t, m) => _post({ group: "utils", action: "notify", title: t, msg: m }),
            beep: () => _post({ group: "utils", action: "beep" }),
            openExternal: (u) => _post({ group: "utils", action: "openExternal", param: u })
        },

        discord: {
            setActivity: (config) => {
                _post({ 
                    group: "discord", action: "setActivity", 
                    details: config.details || "", state: config.state || "", 
                    largeImage: config.largeImage || "", largeText: config.largeText || "", 
                    smallImage: config.smallImage || "", smallText: config.smallText || "", 
                    timer: config.timer === true
                });
            }
        },

        installer: {
            run: () => _post({ group: "installer", action: "run" })
        }
    };
})();
)";