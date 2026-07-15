# Build Organic Maps APK (Windows)


## Regenerate the drawing rules

From the repository root, open PowerShell and run:

```powershell
wsl
cd tools/unix
./generate_drules.sh
```

## Regenerate the map data (.mwm)




```sh
wsl
./tools/unix/build_omim.sh -r generator_tool # Build the `generator_tool` binary (run from the root of the repo)
./tools/unix/build_omim.sh -r world_roads_builder_tool
./tools/unix/build_omim.sh -r mwm_diff_tool
cd tools/python/maps_generator
pip3 install -r requirements_dev.txt
cp var/etc/map_generator.ini.default var/etc/map_generator.ini #Create a [configuration file with defaults](https://github.com/organicmaps/organicmaps/blob/master/tools/python/maps_generator/var/etc/map_generator.ini.default):
```

Read through and edit the configuration file. Ensure that `OMIM_PATH` is set correctly. The default `PLANET_URL` setting makes the generator to download an OpenStreetMap dump file for the North Macedonia from [Geofabrik](http://download.geofabrik.de/index.html). Change `PLANET_URL` and `PLANET_MD5_URL` to get a region you want.

```sh
source .venv/bin/activate
python3 -m maps_generator --countries="US_Washington_Coast, US_Washington_Seattle, US_Washington_Yakima" --skip="Coastline"
```

The `.mwm` files are written under `maps_build/<timestamp>/`. Copy the new `.mwm` files onto the phone, replacing the downloaded ones. Find the app's maps folder (usually in /Android/data/app.organicmaps/files/).


Restart the app afterwards so it picks up the replaced maps.

## Build a new APK

Open PowerShell:

```powershell
cd android
```

(Optional) Remove previous build files:

```powershell
.\gradlew.bat clean
```

Build the release APK:

```powershell
.\gradlew.bat assembleFdroidRelease
```

The APK will be created at:

```
android/app/build/outputs/apk/fdroid/release/
```

Install on phone