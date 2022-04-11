#! /usr/bin/python
import os

IOV_list= ['2016APVP8','2016APVQCD',
           '2016P8','2016QCD',
           '2017P8','2017QCD',
           '2018P8','2018QCD']
version = 'v20'

#os.system("rm *.so *.d *.pcm")
os.system("root -l -b -q mk_CondFormats.C")
for iov in IOV_list:
    print "Process GamHistFill.C+g for IOV "+iov
    os.system("ls -ltrh files/GamHistosFill_mc_"+iov+".root")
    os.system("ls -ltrh files/GamHistosFill_data_"+iov+".root")
    os.system("ls -ltrh log_"+iov+"_"+version+".txt")
    os.system("root -l -b -q 'mk_GamHistosFill.C(\""+iov+"\")' > log_"+iov+"_"+version+".txt &")
#    os.system("fs flush")
#    wait()
#    time.sleep(sleep_time)
