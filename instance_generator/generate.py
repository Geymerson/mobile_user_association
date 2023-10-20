import gap_generator_utils as gen

ues_vec = [100, 200]
enbs_vec = [5, 10, 20]
index = 1
instance_type = [0, 1, 2, 3, 4, 5]

for ues in ues_vec:
    for enbs in enbs_vec:
        ds = gen.DatasetGenerator(ues, enbs, 2, index, instance_type[3]) #(ues, enbs, dcs, ds num, type)
        ds.generate_dataset()
        index+=1
