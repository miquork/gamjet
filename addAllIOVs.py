#! /usr/bin/python
import os

# How to merge files into a bigger one. First one is the target
#IOV_list= ['2018P8','2018A','2018B','2018C','2018D1','2018D2']
IOV_list_of_lists = [
    ['2016BCDEF','2016BCD','2016EF'],
    ['2017BCDEF','2017B','2017C','2017D','2017E','2017F'],
    ['2018ABCD','2018A1','2018A2','2018B','2018C',
     '2018D1','2018D2','2018D3','2018D4'],
    ['Run2','2016BCDEF','2016FGH','2017BCDEF','2018ABCD']
    ]
MC_list_of_lists = [
    ['Run2P8','2016P8','2016APVP8','2017P8','2018P8'],
    ['Run2QCD','2016QCD','2016QCDAPV','2017QCD','2018QCD'],
    ]

version = 'v18'

os.system("ls files/GamHistosFill_data_*_"+version+".root")
for IOV_list in IOV_list_of_lists:
    command = "hadd "
    for iov in IOV_list:
        command = command + "files/GamHistosFill_data_"+iov+"_"+version+".root "
    print "\""+command+"\"..."
    os.system(command)

os.system("ls files/GamHistosFill_mc_*_"+version+".root")
for MC_list in MC_list_of_lists:
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
