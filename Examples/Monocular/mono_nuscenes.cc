/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2020 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
* the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with ORB-SLAM3.
* If not, see <http://www.gnu.org/licenses/>.
*/

#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include <cstdio> 
#include<iomanip>

#include<opencv2/core/core.hpp>
#include"System.h"

int count_i;

using namespace std;

void LoadImages(const string &strSequence, vector<string> &vstrImageFilenames,
                vector<double> &vTimestamps);

void LoadSegment(const string &strSequence, vector<string> &vstrImageFilenames,
                vector<double> &vTimestamps);

int main(int argc, char **argv)
{
    if(argc != 5)
    {
        cerr << endl << "Usage: ./mono_kitti path_to_vocabulary path_to_settings path_to_sequence" << endl;
        return 1;
    }

    bool semantic_mode = false;

    string str = argv[3];
    istringstream iss(str);
    string token;
    while (getline(iss, token, '/'))
    {
        continue;
    }
    cerr<<token<<endl;

    // Retrieve paths to images and segments
    vector<string> vstrImageFilenames;
    vector<string> vstrSegFilenames;
    vector<double> vTimestamps;
    LoadImages(string(argv[3]), vstrImageFilenames, vTimestamps);
    if(semantic_mode){
        LoadSegment(string(argv[4]), vstrSegFilenames, vTimestamps);
    }
    else{
        vstrSegFilenames = vstrImageFilenames;
    }
    cerr<<"after loading";

    int nImages = vstrImageFilenames.size();


    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SLAM(argv[1],argv[2],ORB_SLAM3::System::MONOCULAR,true, semantic_mode);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    // Main loop
    cv::Mat im;
    cv::Mat seg;
    vector <cv::Mat> results;
    int count_zero = 0;
    bool flag = true; 
    for(int ni=0; ni<nImages; ni++)
    {
        // Read image from file

        im = cv::imread(vstrImageFilenames[ni],cv::IMREAD_UNCHANGED);
        if (semantic_mode){
            seg = cv::imread(vstrSegFilenames[ni],-1);
        }
        else{
            seg =-1* cv::Mat::zeros(im.rows, im.cols, CV_8UC1);
        }
        
        double tframe = vTimestamps[ni];

        if(im.empty())
        {
            cerr << endl << "Failed to load image at: " << vstrImageFilenames[ni] << endl;
            return 1;
        }
        if(seg.empty())
        {
            cerr << endl << "Failed to load segment at: " << vstrSegFilenames[ni] << endl;
            return 1;
        }

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the image to the SLAM system
        cv::Mat res = SLAM.TrackMonocular(im,tframe,vector<ORB_SLAM3::IMU::Point>(), vstrImageFilenames[ni]);
        if (res.size().width!=0){
            flag = false;
        }
        if(flag){
            count_zero = count_zero + 1;
        }
#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();

        vTimesTrack[ni]=ttrack;

        // Wait to load the next frame
        double T=0;
        if(ni<nImages-1)
            T = vTimestamps[ni+1]-tframe;
        else if(ni>0)
            T = tframe-vTimestamps[ni-1];
        if(ttrack<T)
            usleep((T-ttrack)*1e6);
    }
    cerr<<count_zero<<endl;
    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        totaltime+=vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
    cout << "mean tracking time: " << totaltime/nImages << endl;

    // Save camera trajectory
    string save_path = "/Datasets/nuScenes/eval/orbslam3/"+token;

    //The first several frames are used as initialization. So we assign a value to it

    //SLAM.SaveKeyFrameTrajectoryTUM(save_path+"/KeyFrameTrajectory_orb.txt");
    SLAM.SaveTrajectoryKITTI(save_path+"/CameraTrajectory_orb.txt");

    ifstream reading(save_path+"/CameraTrajectory_orb.txt");
    string firstline;
    getline(reading, firstline);
    reading.close();

    ifstream reading_new(save_path+"/CameraTrajectory_orb.txt");
    ofstream writing(save_path+"/temp.txt");
    for(int i = 0; i < count_zero; i++){
        writing<<firstline<<"\n";
    }
    writing<<reading_new.rdbuf();
    reading_new.close();
    writing.close();

    string input_file = save_path+"/CameraTrajectory_orb.txt";
    string output_file = save_path+"/temp.txt";
    remove(input_file.c_str());
    rename(output_file.c_str(),input_file.c_str());

    return 0;
}

void LoadImages(const string &strPathToSequence, vector<string> &vstrImageFilenames, vector<double> &vTimestamps)
{
    ifstream fTimes;
    string strPathTimeFile = strPathToSequence + "/times.txt";
    fTimes.open(strPathTimeFile.c_str());
    while(!fTimes.eof())
    {
        string s;
        getline(fTimes,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            ss >> t;
            vTimestamps.push_back(t/1e9);
        }
    }

    string strPrefixLeft = strPathToSequence + "/image_0/";

    const int nTimes = vTimestamps.size();
    vstrImageFilenames.resize(nTimes);

    for(int i=0; i<nTimes; i++)
    {
        stringstream ss;
        ss << setfill('0') << setw(6) << i;
        vstrImageFilenames[i] = strPrefixLeft + ss.str() + ".jpg";
    }
}

void LoadSegment(const string &strPathToSequence, vector<string> &vstrImageFilenames, vector<double> &vTimestamps)
{
    ifstream fTimes;
    string strPathTimeFile = strPathToSequence + "/times.txt";
    fTimes.open(strPathTimeFile.c_str());
    while(!fTimes.eof())
    {
        string s;
        getline(fTimes,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            ss >> t;
            vTimestamps.push_back(t/1e9);
        }
    }

    string strPrefixLeft = strPathToSequence + "/image_0/";

    const int nTimes = vTimestamps.size();
    vstrImageFilenames.resize(nTimes);

    for(int i=0; i<nTimes; i++)
    {
        stringstream ss;
        ss << setfill('0') << setw(6) << i;
        vstrImageFilenames[i] = strPrefixLeft + ss.str() + ".jpg";
    }
}