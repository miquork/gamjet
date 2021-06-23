#! /usr/bin/python
import os

# Run BCDEF first in case using global fit 'dofsrcombo' option
#IOV_list= ['BCDEF','B','C','D','E','F']
#IOV_list= ['2018ABCD','2018A','2018B','2018C','2018D']
#IOV_list= ['2016GH','2016BCDEF','2016BCD','2016EF']
#IOV_list= ['2016BCDEF','2016GH','2016BCD','2016EF']
IOV_list= ['2018P8','2018A','2018B','2018C','2018D1','2018D2']
version = 'v5'
#os.system("rm *.so *.d *.pcm")
os.system("root -l -b -q mk_CondFormats.C")
for iov in IOV_list:
    print "Process GamHistFill.C for IOV "+iov
    os.system("ls -ltrh files/GamHistosFill_mc_"+iov+".root")
    os.system("ls -ltrh files/GamHistosFill_data_"+iov+".root")
    os.system("ls -ltrh log_"+iov+"_"+version+".txt")
    os.system("root -l -b -q 'mk_GamHistosFill.C(\""+iov+"\")' > log_"+iov+"_"+version+".txt &")
#    os.system("fs flush")
#    wait()
#    time.sleep(sleep_time)
