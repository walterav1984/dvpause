#!/bin/sh
#Description:this script adds all *.dv video files in a specific folder as playlist items for dvpause!
#Just setup the following 3 dvp_* variables below and it will run dvpause!

#set dvpause source folder for compilation/execution
dvp_src=~/mygitprojects/dvpause

#set folder which contains dv video files for adding to dvpause playlist
dvp_videos=~/mygitprojects/dvpause/videos/

#set dvswitch connection parameters
dvp_dvswitch='-p 1234 -h 127.0.0.1'
#dvp_dvswitch='' #leave empty if using MIXER_HOST MIXER_PORT from config file ".dvswitchrc"

#fill playlist file with location data
echo "__BASEDIR" > $dvp_src/config
echo $dvp_videos >>  $dvp_src/config

#fill playlist with videofile data
##TODO filter dv extension only?
ls  $dvp_videos | sed 'p;' | sed ': loop; i __MOVIE
n; n; b loop' >> $dvp_src/config

#cd $dvp_src
#make clean
#./configure
#make run

#runs dvpause
$dvp_src/main | dvsource-file - $dvp_dvswitch
