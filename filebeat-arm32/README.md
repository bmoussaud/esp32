FROM https://jschumacher.info/2021/03/up-to-date-filebeat-for-32bit-raspbian-armhf/

docker build -t filebeat-arm32-builder:0.0.0 -f . .
docker run --name builder  filebeat-arm32-builder:0.0.0
docker cp build:/root/Downloads/filebeat_armhf/filebeat-7.11.2-armhf.deb .
docker stop builder
docker rm builder
### Ready: /root/Downloads/filebeat_armhf/filebeat-7.11.2-armhf.deb
scp filebeat-7.11.2-armhf.deb pi@yourpi.local:/tmp
### Deploy on your Raspbian RPi with: 'dpkg -i filebeat-7.11.2-armhf.deb
sudo dpkg -i filebeat-7.11.2-armhf.deb
systemctl status filebeat
systemctl start filebeat
==== AUTHENTICATING FOR org.freedesktop.systemd1.manage-units ===
Authentication is required to start 'filebeat.service'.
Authenticating as: ,,, (pi)
Password:xxxxxx
==== AUTHENTICATION COMPLETE ===
