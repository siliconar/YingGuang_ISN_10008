#ifndef PACK_DEFINITION_H
#define PACK_DEFINITION_H

#include <QString>


struct DataPack_matlab
{
    QString base_name;         // LCVNIR-20250613_144349-00001
    QString input_dat_full;  // path/LCVNIR-20250613_144349-00001.dat
    QString integ_time;     // 18ms
    QString out_folder;      //  path/LCVNIR-20250613_145257-00001
    QString out_png_name_full;       // outpath/LCVNIR-20250613_145257-00001.png
    QString out_L1B_dat_name_full;      // outpath/LCVNIR-20250613_145257-00001_L1B.dat
};

struct DataPack_pos_zx
{
    QString in_dat_full;
    QString out_csv_full;
};

struct DataPack_Jihe
{
    QString basename1;
    QString in_dat_full;
    QString in_csv_full;
    QString out_folder;
    QString out_L_jpg_full;
};


#endif 