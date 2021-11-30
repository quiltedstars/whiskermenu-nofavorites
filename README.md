# whiskermenu-nofavorites
XFCE Whisker Menu without the Favorites section

Works on Void Linux, tested only through git clone.

INSTALL

git clone https://github.com/quiltedstars/whiskermenu-nofavorites.git
cd whiskermenu-nofavorites
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
cmake --install build

UNINSTALL
cmake --build build --target uninstall

TO USE ON XFCE
Right click the panel and click panel > panel preferences, or make your preferred way into panel settings. Add Whisker Menu into Items. 

TO CUSTOMIZE MENU
Right click on menu, click Properties. Or click the cog sign under panel > panel preferences > items > whisker menu. You can remove the Recents tab by putting the max recent to 0 in the Behaviour tab. 
Reorder and customize as usual.
