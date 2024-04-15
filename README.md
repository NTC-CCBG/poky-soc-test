# Poky soc test
This is Nuvoton test bsp layer based on Raspberry pi 4B,which ia an ARM based SoC.
## meta-nuvoton directroy tree
![image](https://github.com/NTC-CCBG/poky-soc-test/blob/main/meta-nuvoton.png)
## Build nuvoton test image
* Clone the poky from Yocto Project
```
  $ git clone -b kirkstone git://git.yoctoproject.org/poky.git
```
enter the poky directroy, then
* Clone the meta-raspberrypi and meta-openembedded
```
git clone -b kirkstone https://github.com/agherzan/meta-raspberrypi.git
git clone -b kirkstone git://git.openembedded.org/meta-openembedded
```
* In poky directory run script
```
$ source oe-init-build-env <build dir name>
$ bitbake-layers add-layer ../meta-raspberrypi ../meta-openembedded ../meta-nuvoton
```
According to your machine to config conf/local.conf, then run
```
$ bitbake nuvoton-image
```
