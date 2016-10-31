struct Elevation_Map;

struct Bt_Loader {
    Bt_Loader();
    ~Bt_Loader();

    Elevation_Map *load(File_Handle *file);
};
