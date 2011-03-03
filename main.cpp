#include <iostream>
#include <QDir>
#include <QDebug>
#include <SensorDataFile.h>
#include "main.h"
#include <TChain.h>
#include <TFile.h>
#include <math.h>
#include <TMessage.h>
#include <QByteArray>
#include <QDate>
#include <QDateTime>
QList<int> sensors;
QList<int> owids;
QList<QString> ownames;

int main(int argc, char** argv) {
    sensors << 34669489
            << 34669968
            << 34675374
            << 34675954
            << 34679387
            << 34684272
            << 34684667
            << 34688908
            << 34690400
            << 34694386
            << 34697439
            << 34699038
            << 34701326
            << 34701412
            << 34703921
            << 34704763
            << 34707079
            << 34713191
            << 34716973
            << 34726045
            << 34727685
            << 34731208
            << 34731514
            << 34745865
            << 34860968
            << 35729408
            << 35729409
            << 35729664
            << 35729665
            << 35730176
            << 35730177
            << 35731200
            << 35731201
            << 35741696
            << 35741697
            << 35741952
            << 35741953
            << 35742208
            << 35742209
            << 35742464
            << 35742465
            << 35742720
            << 35742721
            << 35742976
            << 35742977
            << 35743232
            << 35743233
            << 35743488
            << 35743489
            << 35747840
            << 35747841
            << 35748096
            << 35748097
            << 35748352
            << 35748353
            << 35748608
            << 35748609
            << 35748864
            << 35748865
            << 35749120
            << 35749121
            << 35749376
            << 35749377
            << 35749632
            << 35749633
            << 35750088
            << 35750089
            << 35750090
            << 35750091
            << 35750092
            << 35750093
            << 35750094
            << 35750095
            << 35750097
            << 35848192
            << 35848193
            << 35848194
            << 35848195
            << 36765696
            << 36765697
            << 38862848
            << 37289984;
    owids << 21856
          << 7410
          << 32868
          << 35377
          << 44647
          << 10843
          << 57501
          << 48429
          << 28895
          << 945
          << 25842
          << 38535
          << 59141
          << 6830
          << 30494
          << 62664
          << 1424
          << 16123
          << 36219
          << 15728
          << 32782
          << 20364;
    ownames << QString("TRD_UFE_BOTTOM_COLD_TEMP")
            << QString("TRD_UFE_TOP_COLD_TEMP")
            << QString("TRACKER_UFE_BOTTOM_COLD_TEMP")
            << QString("TRACKER_UFE_TOP_COLD_TEMP")
            << QString("TRACKER_UFE_TOP_HOT_TEMP")
            << QString("TRACKER_UFE_BOTTOM_HOT_TEMP")
            << QString("TRD_UFE_TOP_HOT_TEMP")
            << QString("TRD_UFE_BOTTOM_HOT_TEMP")
            << QString("POWER_UFE_TEMP")
            << QString("USB_BOARD_TEMP")
            << QString("POWER_MID_TEMP")
            << QString("TRD_TUBE_TOP_HOT_TEMP")
            << QString("TRD_TUBE_TOP_COLD_TEMP")
            << QString("TRD_GAS_COLD_TEMP")
            << QString("TRD_GAS_HOT_TEMP")
            << QString("TRD_TUBE_BOTTOM_HOT_TEMP")
            << QString("TRD_TUBE_BOTTOM_COLD_TEMP")
            << QString("POWER_GAS_TEMP")
            << QString("OUTLET_TEMP")
            << QString("INLET_TEMP")
            << QString("BAT_BOTTOM_TEMP")
            << QString("BAT_TOP_TEMP");
    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " directory" << std::endl;
        return -1;
    }
    QDir dir;
    if (!dir.exists(argv[1])) {
        std::cout << "ERROR: directory does not exist" << std::endl;
        return -1;
    }
    dir.setPath(argv[1]);
    QStringList filters;
    filters << "sensors*.dat";
    QStringList files = dir.entryList(filters);
    qDebug() << files;
    int success_count = 0;
    if (!files.size()) {
        std::cout << "Directory doesn't contain any convertable files." << std::endl;
        return -1;
    }
    QList<QString> rootfiles;
    unsigned int lasttime = 0;
    for (int i = 0; i< files.size();i++) {
        QString path = dir.absolutePath();
        qDebug() << path;
        path.append("/");
        qDebug() << path;
        path.append(files[i]);
        qDebug() << path;
        QString rootfile = doConversion(path,lasttime);
        if (!rootfile.isEmpty()) {
            success_count++;
            rootfiles.append(rootfile);
        }
    }
    TChain chain("sensors");
    for (int i = 0; i < rootfiles.length();i++) {
        chain.Add(qPrintable(rootfiles[i]));
    }
    QString resultfile = dir.absolutePath();
    resultfile.append("/sensors.root");


    chain.Merge(qPrintable(resultfile));
    QStringList atcfilter;
    atcfilter << "bexus11_positiondata_table.dat";
    QStringList atcfiles = dir.entryList(atcfilter);
    TFile* resultrootfile = new TFile(qPrintable(resultfile),"UPDATE");
    if (atcfiles.isEmpty()) {
        std::cout << "bexus11_positiondata_table.dat not found";
    } else {

        TTree* atctree = new TTree("ATC","Air Traffic Control data");
        unsigned int atctime;
        //int atcid;
        float atclatitude;
        float atclongitude;
        float atcheight;
        float atchor_speed;
        float atcheading;

        atctree->Branch("time",&atctime);
        //atctree->Branch("ID",&atcid);
        atctree->Branch("ATC_LATITUDE",&atclatitude);
        atctree->Branch("ATC_LONGITUDE", &atclongitude);
        atctree->Branch("ATC_HEIGHT",&atcheight);
        atctree->Branch("ATC_HORIZONTAL_SPEED",&atchor_speed);
        atctree->Branch("ATC_HEADING",&atcheading);
        QString atcfile = dir.absoluteFilePath(atcfiles[0]);
        QFile* file = new QFile(atcfile);
        file->open(QFile::ReadOnly);
        unsigned int previoustime = 0;
        while (!file->atEnd()) {
            QByteArray lineba = file->readLine();
            QString line(lineba);
            if (!line.startsWith('#')) {

                QStringList linedata = line.split(" ");
                atctime = linedata[0].toUInt();
                unsigned int maxtime = atctime;
                if (previoustime != 0 && atctime != previoustime + 1) {
                    for (unsigned int i = previoustime;i<maxtime;i++) {
                        atctime = i;
                        atctree->Fill();
                    }
                }

                //atcid = linedata[1].toInt();
                atclatitude = linedata[2].toFloat();
                atclongitude = linedata[3].toFloat();
                atcheight = linedata[4].toFloat();
                atchor_speed = linedata[5].toFloat();
                atcheading = linedata[6].toFloat();
                atctree->Fill();

            }
        }
        file->close();
        delete file;

    }
    QStringList ebassfilter;
    ebassfilter << "Bexus11.csv";
    QStringList ebassfiles = dir.entryList(ebassfilter);
    if (ebassfiles.isEmpty()) {
        std::cout << "Bexus11.csv not found";
    } else {

        TTree* ebasstree = new TTree("EBASS","EBASS data");
        unsigned int ebasstime;
        float ebasslatitude;
        float ebasslongitude;
        float ebassheight;
        float ebasshor_speed;
        float ebassheading;
        float ebasstemp_out;
        float ebasstemp_gas;
        float ebassair_pres;

        ebasstree->Branch("time", &ebasstime);
        ebasstree->Branch("EBASS_LATITUDE", &ebasslatitude);
        ebasstree->Branch("EBASS_LONGITUDE",&ebasslongitude);
        ebasstree->Branch("EBASS_HEIGHT", &ebassheight);
        ebasstree->Branch("EBASS_HORIZONTAL_SPEED", &ebasshor_speed);
        ebasstree->Branch("EBASS_HEADING", &ebassheading);
        ebasstree->Branch("EBASS_TEMP_OUT",&ebasstemp_out);
        ebasstree->Branch("EBASS_TEMP_GAS", &ebasstemp_gas);
        ebasstree->Branch("EBASS_PRESSURE", &ebassair_pres);
        QString ebassfile = dir.absoluteFilePath(ebassfiles[0]);
        qDebug() << "Ebass: " << ebassfile;
        QFile* file = new QFile(ebassfile);
        file->open(QFile::ReadOnly | QIODevice::Text);

        qDebug() << file->readLine();
        qDebug() << file->readLine();
        QMap<unsigned int, QVector<float> > ebassvalues;
        unsigned int previous = 0.;
        while (!file->atEnd()) {
            QByteArray lineba = file->readLine();
            QString line(lineba);
            QStringList linedata = line.split(";");
            if (linedata[0].isEmpty()) {
                continue;
            }
            QDateTime datetime;
            datetime.setTimeSpec(Qt::UTC);
            datetime.setDate(QDate(2010,11,23));
            linedata[74].chop(1);
            datetime.setTime(QTime::fromString(linedata[74],"hh:mm:ss"));
            unsigned int time = datetime.toTime_t();
            float latitude = ((int)((bool)(linedata[66].contains("N")))*2-1)*(linedata[67].toInt()/100000+((float)(linedata[67].toInt()%100000))/60000.);
            float longitude = ((int)((bool)(linedata[68].contains("E")))*2-1)*(linedata[69].toInt()/100000+((float)(linedata[69].toInt()%100000))/60000.);
            float height = linedata[72].toFloat();
            float horspeed = linedata[70].toFloat();
            float heading = linedata[71].toFloat();
            float temp_out = linedata[3].toFloat();
            float temp_gas = linedata[5].toFloat();
            float press = linedata[7].toFloat();
            unsigned int maxtime = time;
            if (previoustime !=0 && previoustime + 1 != maxtime) {
                for (int i = previoustime; i<maxtime;i++) {
                    ebassvalues[i].push_back(latitude);
                    ebassvalues[i].push_back(latitude);
                    ebassvalues[i].push_back(height);
                    ebassvalues[i].push_back(horspeed);
                    ebassvalues[i].push_back(heading);
                    ebassvalues[i].push_back(temp_out);
                    ebassvalues[i].push_back(temp_gas);
                    ebassvalues[i].push_back(press);
                }
            }
            ebassvalues[time].push_back(latitude);
            ebassvalues[time].push_back(longitude);
            ebassvalues[time].push_back(height);
            ebassvalues[time].push_back(horspeed);
            ebassvalues[time].push_back(heading);
            ebassvalues[time].push_back(temp_out);
            ebassvalues[time].push_back(temp_gas);
            ebassvalues[time].push_back(press);
            qDebug() << "Added for time " << time;// << ": " << ebassvalues[time];
        }
        int previoustime = 0;
        QMapIterator<unsigned int, QVector<float> > j(ebassvalues);
        while (j.hasNext()) {
            j.next();
            if (previoustime != 0 && j.key() != previoustime ) {
                for (int i=previoustime+1;i<j.key();i++) {
                    ebassvalues[i] = ebassvalues[previoustime];
                }
            }
        }
        j.toFront();
        while (j.hasNext()) {
            j.next();
            QVector<float> vec = j.value();
            ebasstime = j.key();
            ebasslatitude = vec.at(0);
            ebasslongitude = vec.at(1);
            ebassheight = vec.at(2);
            ebasshor_speed = vec.at(3);
            ebassheading = vec.at(4);
            ebasstemp_out = vec.at(5);
            ebasstemp_gas = vec.at(6);
            ebassair_pres = vec.at(7);
            ebasstree->Fill();
        }

    }
    resultrootfile->Write();
    if (success_count) {
        std::cout << "Sucessfully converted " << success_count << " files." << std::endl;
    } else {
        std::cout << "Didn't convert any files." << std::endl;
    }
}


QString doConversion(QString path, unsigned int &lasttime) {
    SensorDataFile* file = new SensorDataFile(path,SensorDataFile::MODE_READING,SensorDataFile::TYPE_RAW);
    quint32 currenttime = 0;
    quint32 newlasttime = 0;
    QString rootpath = path;
    qDebug() << rootpath;
    rootpath.chop(3);
    qDebug() << rootpath;
    rootpath.append("root");
    qDebug() << rootpath;
    TFile* rootfile = new TFile(qPrintable(rootpath),"RECREATE","PERDaix flight sensor data",1);
    TTree* tree = new TTree("sensors", "Onboard sensor data");
    QMap<int,float> values;
    QMap<int,float> values2;
    for (int i = 0;i<sensors.length();i++) {
        //qDebug() << i;
        values2[sensors[i]] = 0;
        //qDebug() << hex << sensors[i] << ": " << hex << (sensors[i] & 0xFFF0000);
        if ((sensors[i] == 37289984)) {
            tree->Branch("TRIGGER_RATE",&values2[sensors[i]]);
        }
        if ((sensors[i] & 0xFFF0000) == 0x2210000) {
            if ((sensors[i] & 0x000FFF0) == 0x80c0) {
                tree->Branch(qPrintable(QString("TOF_").append(QString::number((sensors[i] & 0x00000FF)-0xc7,10)).append("_TEMP")),&values2[sensors[i]]);
            } else if ((sensors[i] & 0x000FFF0) == 0x80d0) {
                tree->Branch(qPrintable(QString("TRIGGER_BOARD_TEMP")),&values2[sensors[i]]);
            } else {
                tree->Branch(qPrintable(QString("HPE_0x").append(QString::number(sensors[i] & 0x000FFFF,16)).append("_TEMP")),&values2[sensors[i]]);
            }
        } else {
            if ((sensors[i] & 0x00F0000) == 0x0020000) {
                tree->Branch(qPrintable(QString("TRD_PRESSURE")),&values2[sensors[i]]);
            } else  if ((sensors[i] & 0x00F0000) == 0x0030000) {
                if (sensors[i] == 0x2230000) {
                    tree->Branch("TRACKER_1_VOLTAGE",&values2[sensors[i]]);
                }
                if (sensors[i] == 0x2230001) {
                    tree->Branch("TRACKER_2_VOLTAGE",&values2[sensors[i]]);
                }
                if (sensors[i] == 0x2230002) {
                    tree->Branch("TRACKER_3_VOLTAGE",&values2[sensors[i]]);
                }
                if (sensors[i] == 0x2230003) {
                    tree->Branch("TRD_VOLTAGE",&values2[sensors[i]]);
                }
                if (sensors[i] == 0x213efa8) {
                    tree->Branch("TOF_VOLTAGE",&values2[sensors[i]]);
                }
            } else {
                if ((sensors[i] & 0x0F00000) == 0x0100000) {
                    //tree->Branch(qPrintable(QString("OW_0x").append(QString::number(sensors[i],16))),&values2[sensors[i]]);
                } else if ((sensors[i] & 0x0F00000) == 0x0300000){
                    if (sensors[i] == 0x2310000) {
                        tree->Branch("CPU_TEMP",&values2[sensors[i]]);
                    }
                    if (sensors[i] == 0x2310001) {
                        tree->Branch("PC_TEMP",&values2[sensors[i]]);
                    }

                } else if ((sensors[i] & 0x0F00000) == 0x0500000) {
                    tree->Branch("TDC_TEMP",&values2[sensors[i]]);
                } else {
                    //tree->Branch("TRIGGER_RATE",&values2[sensors[i]]);

                }
            }
        }
    }
    //qDebug() << "hier bin ich";
    for (int i = 0; i < owids.length();i++) {
        int id = 0x2110000;
        id |= owids[i];
        tree->Branch(qPrintable(ownames[i]),&values2[id]);
    }
    tree->Branch("time",&currenttime);
    QMap<unsigned int, QMap<int, float> > sensordata;
    qDebug() << "Filling data";
    while (1) {
        SensorData* data = file->ReadData();
        if (!data) { break;}
        SensorID* id = data->GetSensorID();
        if (id->GetSensorType() == SensorID::SENSOR_TEMPERATURE_C ||
            id->GetSensorType() == SensorID::SENSOR_PRESSURE_HPA ||
            id->GetSensorType() == SensorID::SENSOR_VOLTAGE_V ||
            id->GetSensorType() == SensorID::SENSOR_TRIGGER_RATE_HZ) {
                quint32 time = data->GetTime();
                if (time <= lasttime) { continue;}
                sensordata[time][id->GetID32()] = data->ToFloat32();
        }
        delete data;
    }
    //interpolate data

    if (!sensordata.keys().empty()) {
        QList<unsigned int> keys = sensordata.keys();
        for (int i =0;i< sensors.length();i++) {
            // look for first entry
            quint32 smallesttime = keys.first();
            quint32 firsttime = smallesttime;
            float firstentry = sqrt(-1);
            qDebug() << "Finding first entry";
            for (int j = 0; j<keys.length();j++) {
                if (sensordata[keys[j]].contains(sensors[i])) {
                    firsttime = keys[j];
                    firstentry = sensordata[firsttime][sensors[i]];
                    break;
                }
            }
            // fill up to first entry
            qDebug() << "Filling front";
            for (quint32 j = smallesttime; j<firsttime;j++) {
                sensordata[j][sensors[i]] = firstentry;
            }
            quint32 largesttime = keys.last();
            quint32 lasttime = largesttime;
            float lastentry = sqrt(-1);
            qDebug() << "Finding last entry";
            for (int j = keys.length()-1;j>=0;j--) {
                if (sensordata[keys[j]].contains(sensors[i])) {
                    lasttime = keys[j];
                    lastentry = sensordata[lasttime][sensors[i]];
                    break;
                }
            }
            // fill down to last entry
            qDebug() << "Filling back";
            for (quint32 j = largesttime;j>lasttime;j--) {
                sensordata[j][sensors[i]] = lastentry;
            }
            // interpolate the rest
            quint32 time1 = firsttime;
            quint32 time2 = firsttime;
            qDebug() << "interpoliere";
            for (quint32 j = firsttime; j < lasttime; j++) {
                if (sensordata[j].contains(sensors[i])) {
                    time1 = time2;
                    for (quint32 k = time1+1;k<=lasttime;k++) {
                        if (sensordata[k].contains(sensors[i])) {
                            time2 = k;
                            break;
                        }
                    }
                } else {
                    if (j < time2) {
                        sensordata[j][sensors[i]] = ((float)((time2-j)*sensordata[time1][sensors[i]]+(j-time1)*sensordata[time2][sensors[i]]))/((float)(time2-time1));
                    }
                }
            }

        }
    }

    qDebug() << "Filling tree";
    // loop over all times
    if (!sensordata.keys().empty()) {
        QList<unsigned int> keys = sensordata.keys();
        quint32 smallesttime = keys.first();
        if (lasttime != 0) {
            for (quint32 i = lasttime+1;i<smallesttime;i++) {
                currenttime = i;
                for (int j = 0; j < sensors.length();j++) {
                    values2[sensors[j]] = sqrt(-1);
                }
                tree->Fill();
            }
        }
        quint32 largesttime = keys.last();
        for (quint32 i = smallesttime; i<= largesttime; i++) {
            currenttime = i;
            if (sensordata.contains(i)) {
                QMap<int, float> bla = sensordata[i];
                ;
                for (int j = 0; j < sensors.length();j++) {
                    if (bla.contains(sensors[j])) {
                        values2[sensors[j]] = bla[sensors[j]];
                    } else {
                        values2[sensors[j]] = sqrt(-1);
                    }
                }
            } else {
                QMap<int, float> blam1 = sensordata[i-1];
                for (int j = 0; j < sensors.length();j++) {
                    values2[sensors[j]] = blam1[sensors[j]]; // Fill with previous, shouldn't really happen
                }
            }
            tree->Fill();
        }
        /*while (iter_times.hasNext()) {
        iter_times.next();
        currenttime = iter_times.key();
        // initialize every value to NaN
        QMap<int, float> bla = iter_times.value();
        for (int i = 0; i < sensors.length();i++) {
            if (bla.contains(sensors[i])) {
                values2[sensors[i]] = bla[sensors[i]];
            } else {
                values2[sensors[i]] = sqrt(-1);
            }
        }
        tree->Fill();


    }*/
    }
    tree->Print();
    qDebug() << "Writing tree to file";
    rootfile->Write();
    if (sensordata.keys().empty()) { } else {
        lasttime = sensordata.keys().last();
    }
    rootfile->Close();
    //delete tree;
    delete file;
    return rootpath;







    qDebug() << "Komm ich hier hin?";
}
