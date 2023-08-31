#!/bin/bash
pathDatasetEuroc='/Datasets/nuScenes' #Example, it is necesary to change it by the dataset path

./Monocular/mono_nuscenes ../Vocabulary/ORBvoc.txt "$pathDatasetEuroc"/parser/scene-0002/config.yaml "$pathDatasetEuroc"/parser/scene-0002 "$pathDatasetEuroc"/parser/scene-0002