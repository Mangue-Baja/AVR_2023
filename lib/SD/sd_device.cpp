#include "sd_device.h"

File data;
char file_name[20];

bool init_sd()
{
    if (!SD.begin(SD_CS))
        return false;

    File root = SD.open("/");
    int CountFilesOnSD = 0;

    for (;;)
    {
        File entry = root.openNextFile();
        // no more files
        if (!entry)
            break;

        // for each file count it
        CountFilesOnSD++;
        entry.close();
    }

    sprintf(file_name, "/%s%d.csv", "AV_data", CountFilesOnSD);
    data = SD.open(file_name, FILE_APPEND);

    if (data)
    {
        data.close();
        return true;
    } else
        return false;
}

void save_Data(unsigned long _t30, unsigned long _t100, float _v)
{
    data = SD.open(file_name, FILE_APPEND);

    if (data)
    {
        data.printf("%d,%d,%f\n", _t30, _t100, _v);
        data.close();
    }
}