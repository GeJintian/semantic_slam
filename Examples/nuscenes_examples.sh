#!/bin/bash
pathDatasetEuroc='/Datasets/nuScenes' #Example, it is necesary to change it by the dataset path

./Monocular/mono_nuscenes ../Vocabulary/ORBvoc.txt "$pathDatasetEuroc"/parser/scene-0010/config.yaml "$pathDatasetEuroc"/parser/scene-0010 "$pathDatasetEuroc"/parser/scene-0010