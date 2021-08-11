#! /usr/bin/python
import os

# How to merge files into a bigger one. First one is the target
#IOV_list= ['2018P8','2018A','2018B','2018C','2018D1','2018D2']
IOV_list_of_lists = [
    ['2016BCDEF','2016BCD','2016EF'],
    ['2017BCDEF','2017B','2017C','2017D','2017E','2017F'],
    ['2018ABCD','2018A','2018B','2018C','2018D1','2018D2'],
    ['Run2','2016BCDEF','2016FGH','2017BCDEF','2018ABCD']
    ]
MC_list = ['Run2P8','2016P8','2016P8APV','2017P8','2018P8']

version = 'v11'

os.system("ls files/GamHistosFill_data_*_"+version+".root")
for IOV_list in IOV_list_of_lists:
    command = "hadd "
    for iov in IOV_list:
        command = command + "files/GamHistosFill_data_"+iov+"_"+version+".root "
    print "\""+command+"\"..."
    os.system(command)

os.system("ls files/GamHistosFill_mc_*_"+version+".root")
command = "hadd "
for mc in MC_list:
    command = command + "files/GamHistosFill_mc_"+mc+"_"+version+".root "
print "\""+command+"\"..."
os.system(command)

#for iov in IOV_list:
#    print "Process GamHistFill.C for IOV "+iov
#    os.system("ls -ltrh files/GamHistosFill_mc_"+iov+".root")
#    os.system("ls -ltrh files/GamHistosFill_data_"+iov+".root")
#    os.system("ls -ltrh log_"+iov+"_"+version+".txt")
#    os.system("root -l -b -q 'mk_GamHistosFill.C(\""+iov+"\")' > log_"+iov+"_"+version+".txt &")
#    os.system("fs flush")
#    wait()
#    time.sleep(sleep_time)
