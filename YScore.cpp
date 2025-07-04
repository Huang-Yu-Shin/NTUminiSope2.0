#include "YSCore.h"

namespace NTU {

    YSCore::YSCore(NTU::YSNTUscope* sharedNTUscope) {
        ntuscope = sharedNTUscope;
        mapping = new YSMapping(ntuscope);
        scanning = new YSScan(ntuscope);
    }

    YSCore::~YSCore() {
        delete mapping;
        delete scanning;
    }

    void YSCore::readOffsetFromFile(int selectedIndex) {
        std::string filePath = "C:\\Ian\\MTPFM\\Calibrations\\headMount_YS\\headMount_" + std::to_string(selectedIndex) + ".txt";

        std::ifstream file(filePath);
        if (!file.is_open()) {
            cout << "ERROR: Load file fail: " << filePath << std::endl;
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key;
            char equalSign;
            double value;

            if (iss >> key >> equalSign >> value) {
                if (key == "xOFFSET" && equalSign == '=') {
                    ntuscope->xoffset = value;
                }
                else if (key == "yOFFSET" && equalSign == '=') {
                    ntuscope->yoffset = value;
                }
            }
        }
    }

    void YSCore::initialize() {

        if (!scanning->initialize()) {
            cout << "Initialize NI (include MEMS signal and trigger and MEMS clock) : ERROR" << endl;
        }
        readconfigure();
    }

    void YSCore::FLsetONLY(double voltage) {
        ntuscope->zoffset = voltage;
        if (scanning->ETLfocalset()) {
            cout << "Set focal length : Success" << endl;
        }
    }

    void YSCore::TWODLissajous_configure(double focaloffset, double MEMSXamplitude, double MEMSYamplitude, double midx, double midy, double xfreq, double yfreq, int clocktype) {
        ntuscope->clocktype = clocktype;
        ntuscope->MEMS_xamp = MEMSXamplitude;
        ntuscope->MEMS_yamp = MEMSYamplitude;
        ntuscope->xfreq = xfreq;
        ntuscope->yfreq = yfreq;
        ntuscope->xoffset = midx;
        ntuscope->yoffset = midy;
        ntuscope->num_lines = 0;
        ntuscope->zoffset = focaloffset;
        ntuscope->zstep = 0;
        ntuscope->zlayer = 0;
        if (mapping->ConfigureBoard(mapping->LISSAJOUS_2D)) {
            cout << "Configure Alazar : Success" << endl;
            cout << "---------------------" << endl;
        }
        if (scanning->twoD_lissajous_configure()) {
            cout << "Configure NIDAQ and ETL : Success" << endl;
            cout << "---------------------" << endl;
        }
        if (mapping->Generate2D_LISSAJOUS_SampleMap()) {
            cout << "Configure SampleMap : Success" << endl;
            cout << "---------------------" << endl;
        }
    }

    void YSCore::TWODlissajousgetimage() {
        if (mapping->StartAcquireData()) {
            cout << "Start Alazar data: Success" << endl;
        }
        thread processingThreads = thread(&YSMapping::Trensientstep, mapping);
        Sleep(500);
        thread mappingThread = thread(&YSMapping::AcquireData, mapping);
        thread scanningThread = thread(&YSScan::twoD_lissajous_run, scanning);
        thread avgThread = thread(&YSMapping::AverageFunction, mapping);
        thread repairThread = thread(&YSMapping::RepairFunction, mapping);
        thread saveThread = thread(&YSMapping::SavelissajousimageFunction, mapping);

        mappingThread.join();
        scanningThread.join();
        processingThreads.join();
        avgThread.join();
        repairThread.join();
        saveThread.join();

        mapping->CloseAlazar();
        scanning->stop();

    }

    void YSCore::TWODRaster_configure(double focaloffset, double xfreq, double MEMSXamplitude, double MEMSYamplitude, int num_lines, double midx, double midy, int clocktype) {
        ntuscope->clocktype = clocktype;
        ntuscope->MEMS_xamp = MEMSXamplitude;
        ntuscope->MEMS_yamp = MEMSYamplitude;
        ntuscope->xfreq = xfreq;
        ntuscope->yfreq = 0;
        ntuscope->xoffset = midx;
        ntuscope->yoffset = midy;
        ntuscope->num_lines = num_lines;
        ntuscope->zoffset = focaloffset;
        ntuscope->zstep = 0;
        ntuscope->zlayer = 0;
        if (mapping->ConfigureBoard(mapping->RASTER)) {
            cout << "Configure Alazar : Success" << endl;
            cout << "---------------------" << endl;
        }
        if (scanning->twoD_raster_configure()) {
            cout << "Configure NIDAQ and ETL : Success" << endl;
            cout << "---------------------" << endl;
        }
    }

    void YSCore::TWODRaster_slow_configure(double focaloffset, double xfreq, double MEMSXamplitude, double MEMSYamplitude, int num_lines, double midx, double midy, int clocktype) {
        ntuscope->clocktype = clocktype;
        ntuscope->MEMS_xamp = MEMSXamplitude;
        ntuscope->MEMS_yamp = MEMSYamplitude;
        ntuscope->xfreq = xfreq;
        ntuscope->yfreq = 0;
        ntuscope->xoffset = midx;
        ntuscope->yoffset = midy;
        ntuscope->num_lines = num_lines;
        ntuscope->zoffset = focaloffset;
        ntuscope->zstep = 0;
        ntuscope->zlayer = 0;
        if (mapping->ConfigureBoard(mapping->RASTER_SLOW)) {
            cout << "Configure Alazar : Success" << endl;
            cout << "---------------------" << endl;
        }
        if (scanning->twoD_raster_slow_configure()) {
            cout << "Configure NIDAQ and ETL : Success" << endl;
            cout << "---------------------" << endl;
        }
    }

    void YSCore::TWODrasterimage() {
        if (mapping->StartAcquireData()) {
            cout << "Start Alazar data: Success" << endl;
        }
        thread processingThreads = thread(&YSMapping::Trensientstep, mapping);
        Sleep(500);
        thread mappingThread = thread(&YSMapping::AcquireData, mapping);
        thread scanningThread = thread(&YSScan::twoD_raster_run, scanning);
        thread avgThread = thread(&YSMapping::AverageFunction, mapping);
        thread saveThread = thread(&YSMapping::SaverasterimageFunction, mapping);

        mappingThread.join();
        scanningThread.join();
        processingThreads.join();
        avgThread.join();
        saveThread.join();

        mapping->CloseAlazar();
        scanning->stop();

    }

    void YSCore::stopAllOperations() {
        scanning->running = false;
        mapping->running = false;
    }

    void YSCore::Alignmentsetup(double xoffset, double yoffset) {
        ntuscope->xoffset = xoffset;
        ntuscope->yoffset = yoffset;
        scanning->alignment();
    }

    void YSCore::stopAlignment() {
        scanning->alignment_stop();
    }

    void YSCore::saveimg(int save_num) {
        if (save_num > 0) {
            mapping->imageIndex = 0;
            mapping->SavedCount = 0;
            ntuscope->savestate = true;
            ntuscope->save_num = save_num;
        }
        else {
            cout << "Error: the number of image is incorrect" << endl;
        }
    }

    void YSCore::averagefun(bool run, int avgcount) {
        ntuscope->avgcount = avgcount;
        if (run = true) {
            ntuscope->avgstate = true;
        }
        else if (run = false) {
            ntuscope->avgstate = false;
        }

    }

    void YSCore::readconfigure() {
        ifstream inputFile("C:\\Ian\\MTPFM\\Calibrations\\default_settings\\default_Settings_Driver.txt");
        if (!inputFile.is_open()) {
            cerr << "Error: Cannot open file default_Settings_Driver.txt" << endl;
            return;
        }

        string line;
        while (getline(inputFile, line)) {
            line.erase(remove(line.begin(), line.end(), ' '), line.end());
            istringstream iss(line);
            string key, value;

            getline(iss, key, ':');
            getline(iss, value);

            if (key == "zpixelnumber") {
                try { mapping->Zpixelnum = int(stod(value)); }
                catch (const exception& e) {
                    cerr << "Error converting zpixelnumber value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "MEMSfilter") {
                try { scanning->MEMScutofffreq = int(stod(value)); 
                }
                catch (const exception& e) {
                    cerr << "Error converting MEMSfilter value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "MEMSfilter_high") {
                try {
                    scanning->MEMScutofffreq_high = int(stod(value));
                }
                catch (const exception& e) {
                    cerr << "Error converting MEMSfilter_high value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "MEMSampratio(X/Y)") {
                try { scanning->MEMSampratio = stod(value); }
                catch (const exception& e) {
                    cerr << "Error converting MEMSampratio(X / Y) value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "NItriggerdelay") {
                try { scanning->NItriggerdelay = stod(value); }
                catch (const exception& e) {
                    cerr << "Error converting NItriggerdelay value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "NItriggerdelay_high") {
                try { scanning->NItriggerdelay_high = stod(value); }
                catch (const exception& e) {
                    cerr << "Error converting NItriggerdelay_high value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "AlazarpreTriggerSamples") {
                try { mapping->preTriggerSamples = U32(stod(value)); }
                catch (const exception& e) {
                    cerr << "Error converting AlazarpreTriggerSamples value: " << value << " (" << e.what() << ")" << endl;
                }
            }
            else if (key == "Alazartriggerdelay") {
                try { mapping->Alazartriggerdelay = stod(value); }
                catch (const exception& e) {
                    cerr << "Error converting Alazartriggerdelay value: " << value << " (" << e.what() << ")" << endl;
                }
            }

        }
        inputFile.close();

        cout << "Configure parameter---------------" << endl;
        cout << "zPixelnumber : " << mapping->Zpixelnum << endl;
        cout << "MEMSfilter (Hz): " << scanning->MEMScutofffreq << endl;
        cout << "MEMSfilter_high (Hz): " << scanning->MEMScutofffreq_high << endl;
        cout << "MEMSampratio(X / Y): " << scanning->MEMSampratio << endl;
        cout << "NItriggerdelay (s): " << scanning->NItriggerdelay << endl;
        cout << "NItriggerdelay_high (s): " << scanning->NItriggerdelay_high << endl;
        cout << "AlazarpreTriggerSamples: " << mapping->preTriggerSamples << endl;
        cout << "Alazartriggerdelay (s): " << mapping->Alazartriggerdelay << endl;
        cout << "---------------------" << endl;
    }


}
