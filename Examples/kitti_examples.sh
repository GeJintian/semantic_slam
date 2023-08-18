#!/bin/bash
pathDatasetEuroc='/Datasets/Kitti' #Example, it is necesary to change it by the dataset path

./Monocular/mono_kitti ../Vocabulary/ORBvoc.txt ./Monocular/KITTI04-12.yaml "$pathDatasetEuroc"/data_odometry_gray/dataset/sequences/09 "$pathDatasetEuroc"/data_odometry_gray/dataset/segments/09

#./Stereo/stereo_kitti ../Vocabulary/ORBvoc.txt ./Stereo/KITTI03.yaml "$pathDatasetEuroc"/data_odometry_gray/dataset/sequences/03