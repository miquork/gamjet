#! /usr/bin/python
import os

##################
# Run 2 IOV list #
##################
#IOV_list= ['2016BCD','2016EF','2016FGH',
#           '2017B','2017C','2017D','2017E','2017F',
#           '2018A1','2018A2','2018B','2018C',
#           '2018D1','2018D2','2018D3','2018D4']
#version = 'v20'


##################
# Run 3 IOV list #
##################
IOV_list= ['2022C','2022D',
           '2022E','2022F','2022G',
           '2023B','2023Cv123','2023Cv4']
#           '2023B','2023Cv123','2023Cv4','2023D']
version = 'v21e'

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
