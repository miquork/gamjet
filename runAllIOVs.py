#! /usr/bin/python
import os

# Run BCDEF first in case using global fit 'dofsrcombo' option
#IOV_list= ['BCDEF','B','C','D','E','F']
#IOV_list= ['2018ABCD','2018A','2018B','2018C','2018D']
#IOV_list= ['2017P8','2017B','2017C','2017D','2017E','2017F','2016P8APV']
#IOV_list= ['2016GH','2016BCDEF','2016BCD','2016EF']
#IOV_list= ['2016BCDEF','2016GH','2016BCD','2016EF']
#IOV_list= ['2018P8','2018A','2018B','2018C','2018D1','2018D2']
#IOV_list= ['2018D1','2018D2']
#IOV_list= ['2016P8','2016BCD','2016EF','2016FGH']
#IOV_list= ['2016P8','2016QCD','2016FGH']
#IOV_list= ['2016P8','2016BCD','2016EF','2016FGH',
#           '2017P8','2017B','2017C','2017D','2017E','2017F']
#IOV_list= ['2016BCD','2016EF','2016FGH',
#           '2017B','2017C','2017D','2017E','2017F']
#IOV_list= ['2017C']
#IOV_list= ['2016P8','2017P8']
#IOV_list= ['2016P8APV']
IOV_list= ['2016P8','2016QCD','2016BCD','2016EF',
           '2016APVP8','2016APVQCD','2016FGH',
           '2017P8','2017QCD','2017B','2017C','2017D','2017E','2017F',
           '2018P8','2018QCD','2018A1','2018A2','2018B','2018C',
           '2018D1','2018D2','2018D3','2018D4']
#IOV_list= ['2018A1','2018A2','2018D3','2018D4']
version = 'v19'

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
