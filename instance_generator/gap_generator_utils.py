import logging
import numpy as np
from termcolor import colored

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)
np.set_printoptions(precision=2)

class DatasetGenerator():
	def __init__(self, ues, enbs, dcs, index, tpe):
		self.enbs = enbs
		self.ues = ues
		self.dcs = dcs
		self.index = index
		self.type = tpe
		self.rnd_lower = 0
		self.rnd_upper = 1
		self.enb_dc_cost = np.random.uniform(low=0.1, high=1.0, size=(enbs, dcs))
		self.hand_freq = np.zeros((enbs, enbs))
		self.hi_avg = np.zeros(enbs)
		self.rsrq = np.random.uniform(low=0.1, high=1.0, size=(enbs, ues))
		self.dist = np.random.uniform(low=0.0, high=1.0, size=(enbs, ues))
		self.Wi = np.random.uniform(low=0.3, high=0.6, size=enbs)
		self.users_bdw_requirements = np.random.randint(5, 26,size=(enbs, ues))
		self.enbs_max_bdw =  np.zeros(enbs)
		self.Ji = [[] for i in range(enbs)]
		self.RMax = 0

		self.compute_handover_freq_and_avg()
		self.compute_Ji()
		self.compute_RMax()
		self.compute_enbs_max_bdw()

	def initial_instance(self):
		self.enb_dc_cost = np.ones((self.enbs, self.dcs))
		self.hand_freq = np.zeros((self.enbs, self.enbs))
		self.hi_avg = np.ones(self.enbs)
		self.rsrq = np.ones((self.enbs, self.ues))
		self.dist = np.ones((self.enbs, self.ues))
		self.Wi = np.random.uniform(low=0.3, high=0.6, size=self.enbs)
		self.users_bdw_requirements = np.random.randint(5, 26,size=(self.enbs, self.ues))
		self.enbs_max_bdw =  np.full(self.enbs, 10000)

	def get_user_cost(self, i, k):
		return self.rsrq[i, k] + self.dist[i, k] + self.hi_avg[i] + self.Wi[i]
		
	def compute_handover_freq_and_avg(self):
		for i in range(0, self.enbs):
			sum = 0
			for j in range(0, self.enbs):
				if(i == j):		
					self.hand_freq[i][j] = 0
				elif(j > i):
					self.hand_freq[i][j] = np.random.uniform(0, 1)
				else:
					self.hand_freq[i][j] = self.hand_freq[j][i]
				sum += self.hand_freq[i][j]
			if(self.enbs > 1):
				self.hi_avg[i] = sum/(self.enbs-1)
			else:
				self.hi_avg[i] = 0

	def set_type4_user_requirement(self):
		for i in range(0, self.enbs):
			for k in range(0, self.ues):
				delta = np.random.uniform(-1, 1)
				self.users_bdw_requirements[i, k] = 6 + delta - self.get_user_cost(i, k)

	def set_type5_user_requirement(self):
		for i in range(0, self.enbs):
			for k in range(0, self.ues):
				self.users_bdw_requirements[i, k] = 1

	def compute_Ji(self):
		for k in range(0, self.ues):
			max = -1
			enb = -1
			for i in range(0, self.enbs):
				val = self.get_user_cost(i, k) 
				#print(val)
				if(val > max):
					max = val
					enb = i
			self.Ji[enb].append(k)

	def compute_RMax(self):
		sum = 0
		for k in range(0, self.ues):
			ue_max_req = -1			
			for i in range(0, self.enbs):
				val = self.users_bdw_requirements[i][k] 
				if(val > ue_max_req):
					ue_max_req = val			
			sum += ue_max_req
		self.RMax = sum

	def compute_enbs_max_bdw(self):
		bdw = 0.6*(self.ues/self.enbs)*15 + 0.4*self.RMax

		if(self.type == 0):
			self.initial_instance()
		elif(self.type == 1):
			for i in range (0, self.enbs):
				self.enbs_max_bdw[i] = bdw
		elif(self.type == 2):
			for i in range (0, self.enbs):
				self.enbs_max_bdw[i] = 0.7*bdw
		elif(self.type == 3):
			for i in range (0, self.enbs):
				sum = 0
				for k in range(0, self.ues):
					sum += self.users_bdw_requirements[i][k]
				self.enbs_max_bdw[i] = (sum/self.enbs)*0.8
				#self.enbs_max_bdw[i] = (sum/self.enbs)
		elif(self.type == 4):
			self.set_type4_user_requirement()
			for i in range(0, self.enbs):
				sum = 0
				for k in range(0, self.ues):
					sum += self.users_bdw_requirements[i][k]
				self.enbs_max_bdw[i] = (sum/self.enbs)*0.8
		elif(self.type == 5):
			self.set_type5_user_requirement()
			for i in range(0, self.enbs):
				self.enbs_max_bdw[i] = self.ues*1.2


	def generate_dataset(self):
		print()
		print("Generating type " +str(self.type)+ " datasets for <" + str(self.ues) + "> users, <" 
				+ str(self.enbs)+ "> eNBs, and <" + str(self.dcs)+ "> dcs." )

		f = open("instances/ds"+str(self.index)+".in", "w")
		f.write(str(self.ues)+' '+str(self.enbs)+' '+str(self.dcs)+' '+str(self.type)+' '+str(self.RMax)+'\n')

		print("Saving base stations max bandwidth.")
		for i in range(0, self.enbs):
			f.write(str(round(self.enbs_max_bdw[i], 2)))
			if(i != self.enbs - 1):
				f.write(' ')
		f.write('\n')

		print("Saving users bandwidth requirements.")
		for i in range(0, self.enbs):
			for k in range(0, self.ues):
				f.write(str(self.users_bdw_requirements[i][k]))
				if(k != self.ues - 1):
					f.write(' ')
			f.write('\n')

		print("Saving base stations handover frequency.")
		for i in range(0, self.enbs):
			for j in range(0, self.enbs):
				f.write(str(round(self.hand_freq[i][j], 2)))
				if(j != self.enbs - 1):
					f.write(' ')
			f.write('\n')

		print("Saving base stations current available bandwidth.")
		for i in range(0, self.enbs):
			f.write(str(round(self.Wi[i], 2)))
			if(i != self.enbs - 1):
				f.write(' ')
		f.write('\n')

		print("Saving eNB and data center communication cost.")	
		for i in range(0, self.enbs):
			for s in range(0, self.dcs):
				f.write(str(round(self.enb_dc_cost[i][s], 2)))				
				if(s != self.dcs - 1):
					f.write(' ')
			f.write('\n')
		
		print("Saving base station and user distance.")	
		for i in range(0, self.enbs):
			for k in range(0, self.ues):						
				f.write(str(round(self.dist[i][k], 2)))			
				if(k != self.ues - 1):
					f.write(' ')
			f.write('\n')		
		
		print("Saving base station and user rsrq.")	
		for i in range(0, self.enbs):
			for k in range(0, self.ues):						
				f.write(str(round(self.rsrq[i][k], 2)))			
				if(k != self.ues - 1):
					f.write(' ')
			f.write('\n')
		
		print(colored("Dataset successfully generated.", 'green'),'\n')
		f.close()