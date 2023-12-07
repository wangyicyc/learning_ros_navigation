# learning_ros_navigation
鄙人在学习ros_navigation的过程中深感学习的琐碎，故而分享自己的学习路线以助后人免遭此灾。 
1.（局部视野导航）--DWA 
(1)准备步骤：
 创建六个yaml参数文件：
  1.costmap_common_params.yaml ：为global_costmap & local_costmap共有文件，需利用namespace分别加载到各自地图中
  2.global_costmap_params.yaml ： 全局代价地图的设置
  3.local_costmap_params.yaml ： 局部代价地图的设置
  4.move_base_params.yaml ： 控制器move_base 本身的设置   
  5.DWA_local_planner_params.yaml 
  前三个参数是配置代价地图相关参数，而后面三个是配置路径规划相关参数。 
(2)Yaml文件配置：
  1.加载costmap_common_params.yaml到global_costmap和local_costmap两个命名空间中，
  2.costmap_common_params.yaml的作用：用于配置代价地图，代价地图指将地图中的一个个cell赋值来判定为障碍物，空或未知；
