#ifndef ASSETS_H
#define ASSETS_H

#include <iostream>
#include <filesystem>
#include <fstream>
#include "./config.h"

namespace fs = std::filesystem;

/**
 * @class AssetManager
 * @description Copia los archivos y recursos necesarios para el despliegue del motor.
 */
class AssetManager
{
public:
    static void syncAssets(const WindowConfig &config)
    {
        try
        {
            if (!fs::exists("genesis"))
            {
                fs::create_directory("genesis");
            }

            if (!config.icon.empty() && fs::exists(config.icon))
            {
                fs::copy(config.icon, "genesis/app.ico", fs::copy_options::overwrite_existing);
            }
        }
        catch (const fs::filesystem_error &e)
        {
            std::cerr << "Error al copiar assets: " << e.what() << std::endl;
        }
    }
};

#endif