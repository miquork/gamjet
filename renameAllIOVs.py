#! /usr/bin/python
import os

# Run BCDEF first in case using global fit 'dofsrcombo' option
#IOV_list= ['BCDEF','B','C','D','E','F']
#IOV_list= ['2018ABCD','2018A','2018B','2018C','2018D']
#IOV_list= ['2017P8','2017B','2017C','2017D','2017E','2017F','2016APVP8']
#IOV_list= ['2016GH','2016BCDEF','2016BCD','2016EF']
#IOV_list= ['2016BCDEF','2016GH','2016BCD','2016EF']
#IOV_list= ['2018P8','2018A','2018B','2018C','2018D1','2018D2']
#IOV_list= ['2018D1','2018D2']
#IOV_list= ['2016P8','2016BCD','2016EF','2016FGH']
#IOV_list= ['2016P8','2016BCD','2016EF','2016FGH',
#           '2017P8','2017B','2017C','2017D','2017E','2017F']
#IOV_list= ['2016BCD','2016EF','2016FGH',
#           '2017B','2017C','2017D','2017E','2017F']
#IOV_list= ['2017C']
#IOV_list= ['2016P8','2017P8']
#IOV_list= ['2016APVP8']

##################
# Run 2 IOV list #
##################
#IOV_list= ['2016P8','2016BCD','2016EF',
#           '2016APVP8','2016FGH',
#           '2017P8','2017QCD','2017B','2017C','2017D','2017E','2017F',
#           '2018P8','2018QCD','2018A','2018A1','2018A2',
#           '2018B','2018C',
#           '2018D1','2018D2','2018D3','2018D4',
#           '2016QCD','2016APVQCD','2017QCD','2018QCD']
#version = 'v20'

##################
# Run 3 IOV list #
##################
#IOV_list= ['2022C','2022D',
IOV_list= ['2022P8','2022C','2022D',
           '2022EEP8','2022E','2022F','2022G',
           '2023B','2023Cv123','2023Cv4']
#           '2023B','2023Cv123','2023Cv4','2023D']
version = 'v21'

for iov in IOV_list:
    os.system("mv -i files/GamHistosFill_mc_"+iov+".root files/GamHistosFill_mc_"+iov+"_"+version+".root")
    os.system("mv -i files/GamHistosFill_data_"+iov+".root files/GamHistosFill_data_"+iov+"_"+version+".root")
os.system("ls files/GamHistosFill_*_"+iov+"_"+version+".root")
