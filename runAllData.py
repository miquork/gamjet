#! /usr/bin/python
import os

IOV_list= ['2016BCD','2016EF','2016FGH',
           '2017B','2017C','2017D','2017E','2017F',
           '2018A1','2018A2','2018B','2018C',
           '2018D1','2018D2','2018D3','2018D4']
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
