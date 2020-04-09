# ukui-session-manager

* ukui-session: Launched at the beginning of UKUI sessions and responsible for launching and monitoring all other components constituing the sessions.

* ukui-session-tool: Deals with 'logout', 'shutdown', 'reboot', 'hibernate' and so on.

## Build & Install
### cmake
```
cd ukui-session-manager
mkdir build
cd build
cmake ..
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
