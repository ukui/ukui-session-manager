# ukui-session-manager

![build](https://github.com/ukui/ukui-session-manager/workflows/Check%20build/badge.svg?branch=master)

* ukui-session: Launched at the beginning of UKUI sessions and responsible for launching and monitoring all other components constituing the sessions.

* ukui-session-tool: Deals with 'logout', 'shutdown', 'reboot', 'hibernate' and so on.

## Build & Install
### cmake
```
cd ukui-session-manager
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_SYSCONFDIR=/etc -DCMAKE_INSTALL_LOCALSTATEDIR=/var -DCMAKE_INSTALL_RUNSTATEDIR=/run "-GUnix Makefiles" -DCMAKE_INSTALL_LIBDIR=lib/x86_64-linux-gnu -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
sudo make install
```

### debuild
```
cd ukui-session-manager
uscan --download-current-version # or you can change the word in debian/source/format from 'quilt' to 'native'
debuild
cd ..
sudo dpkg -i ukui-session-manager_{version}_{arch}.deb
```

## Issues
[ukui-session-manager issues](https://github.com/ukui/ukui-session-manager/issues)
