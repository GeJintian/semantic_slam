from nuscenes.nuscenes import NuScenes
import yaml
from shutil import copyfile
import os
from scipy.spatial.transform import Rotation as R


root = '/home/oscar/workspace/orbslam3_docker-main/Datasets/nuScenes/v1.0-trainval/v1.0-trainval01_blobs'
nusc = NuScenes(version='v1.0-trainval', dataroot=root, verbose=True)

my_scene = nusc.scene[1]
name = my_scene['name']
first_sample_token = my_scene['first_sample_token']
my_sample = nusc.get('sample', first_sample_token)

sensor = 'CAM_FRONT'
cam_token = nusc.get('sample_data', my_sample['data'][sensor])['calibrated_sensor_token']
cam_data = nusc.get('calibrated_sensor',cam_token)['camera_intrinsic']
fx = cam_data[0][0]
fy = cam_data[1][1]
cx = cam_data[0][2]
cy = cam_data[1][2]

width = 1600
height = 900


result = {
'Camera.type': "PinHole",

# Camera calibration and distortion parameters (OpenCV) 
'Camera.fx': fx,
'Camera.fy': fy,
'Camera.cx': cx,
'Camera.cy': cy,

'Camera.k1': 0.0,
'Camera.k2': 0.0,
'Camera.p1': 0.0,
'Camera.p2': 0.0,

# Camera frames per second 
'Camera.fps': 12.0,

# Color order of the images (0: BGR, 1: RGB. It is ignored if images are grayscale)
'Camera.RGB': 1,

# Camera resolution
'Camera.width': 1600,
'Camera.height': 900,

#--------------------------------------------------------------------------------------------
# ORB Parameters
#--------------------------------------------------------------------------------------------

# ORB Extractor: Number of features per image
'ORBextractor.nFeatures': 2000,

# ORB Extractor: Scale factor between levels in the scale pyramid 	
'ORBextractor.scaleFactor': 1.2,

# ORB Extractor: Number of levels in the scale pyramid	
'ORBextractor.nLevels': 8,

# ORB Extractor: Fast threshold
# Image is divided in a grid. At each cell FAST are extracted imposing a minimum response.
# Firstly we impose iniThFAST. If no corners are detected we impose a lower value minThFAST
# You can lower these values if your images have low contrast			
'ORBextractor.iniThFAST': 20,
'ORBextractor.minThFAST': 7,

#--------------------------------------------------------------------------------------------
# Viewer Parameters
#--------------------------------------------------------------------------------------------
'Viewer.KeyFrameSize': 0.1,
'Viewer.KeyFrameLineWidth': 1,
'Viewer.GraphLineWidth': 1,
'Viewer.PointSize': 2,
'Viewer.CameraSize': 0.15,
'Viewer.CameraLineWidth': 2,
'Viewer.ViewpointX': 0,
'Viewer.ViewpointY': -10,
'Viewer.ViewpointZ': -0.1,
'Viewer.ViewpointF': 2000
}

save_path = '/home/oscar/workspace/orbslam3_docker-main/Datasets/nuScenes/parser/'+name+'/'

if not os.path.exists(save_path):
    os.makedirs(save_path)
    os.makedirs(save_path+'image_0')
    os.makedirs(save_path+'image_1')

with open(save_path+'config.yaml', 'w', encoding='utf-8') as f:
   yaml.dump(data=result, stream=f, allow_unicode=True)

with open(save_path+'config.yaml','r+') as f:
   content = f.read()
   f.seek(0,0)
   f.write('%YAML:1.0\n'+content)


count = 0
my_sample = nusc.get('sample', first_sample_token)
current = my_sample['data'][sensor]

f = open(save_path+'times.txt','w')
d = open(save_path+'poses.txt','w')
while current!='':
    left_data = nusc.get('sample_data', current)
    pose = nusc.get('ego_pose',left_data['ego_pose_token'])
    file_path = root + '/'+left_data['filename']
    copyfile(file_path,save_path+'image_0/'+str(count).zfill(6)+'.jpg')
    f.write(str(left_data['timestamp'])+'\n')

    r = R.from_quat(pose['rotation']).as_matrix()
    pose_str = str(format(r[0][0],'.6e')) + ' ' + str(format(r[0][1],'.6e')) + ' ' + str(format(r[0][2],'.6e')) + ' ' +str(format(pose['translation'][0],'.6e')) + ' ' + str(format(r[2][0],'.6e')) + ' '  + str(format(r[2][1],'.6e')) + ' ' + str(format(r[2][2],'.6e')) + ' '+  str(format(pose['translation'][2],'.6e')) + ' '  + str(format(r[1][0],'.6e'))  +' ' + str(format(r[1][1],'.6e'))  +' ' + str(format(r[1][2],'.6e')) + ' '+  str(format(pose['translation'][1],'.6e'))
    d.write(pose_str+'\n')
    current = left_data['next']
    count = count + 1
f.close()
d.close()