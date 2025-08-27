#打开libs/arm64-v8a文件夹二进制文件
#移动到/data/lost+found文件夹命名ImGuiNew
cp -f libs/arm64-v8a/* /data/lost+found/local_c4d
#为ImGuiNew文件777权限
chmod 777 /data/lost+found/local_c4d
#运行ImGuiNew
/data/lost+found/local_c4d