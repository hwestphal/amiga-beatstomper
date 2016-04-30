$script = <<SCRIPT
apt-get -q -y update
apt-get -q -y install git g++ bison ncurses-dev python-dev gperf
cd /tmp
git clone git://github.com/cahirwpz/m68k-amigaos-toolchain.git
cd m68k-amigaos-toolchain
./toolchain-m68k --prefix=/opt/m68k-amigaos build
find /opt/m68k-amigaos -type f -perm 600 -exec chmod 644 '{}' ';'
echo 'export PATH=$PATH:/opt/m68k-amigaos/bin' >/etc/profile.d/m68k-amigaos.sh
SCRIPT

Vagrant.configure("2") do |config|
  config.vm.box = "box-cutter/ubuntu1404-i386"
  config.vm.provision "shell", inline: $script
end
