from config_location import LocationConfig
from torch.utils.data import DataLoader
from data import LocationDataset
from inspect import getsource
from torchnet import meter
from tqdm import tqdm
import numpy as np
import torch as t
import models
import utils
import fire
import time
import csv
import os

opt = LocationConfig()
log = utils.log
    
def train(**kwargs):
    name = time.strftime('location_train_%Y%m%d_%H%M%S')
    log_file = open(f"{opt.save_log_root}/{name}.txt", 'w')

    opt.parse(kwargs, log_file)
    start_time = time.strftime("%b %d %Y %H:%M:%S")
    log(log_file, f'Training start time: {start_time}')

    # step1: configure model
    log(log_file, 'Building model...')
    model = models.model(
        module_name=opt.module_name,
        model_name=opt.model_name,
        input_channel=utils.num_category+4,
        output_channel=utils.num_category+3, 
        pretrained=True
    )
    input_channel = 512
    connect = models.connect(
        module_name=opt.module_name,
        model_name=opt.model_name,
        input_channel=input_channel, 
        output_channel=utils.num_category+3,
        reshape=False
    )

    if opt.multi_GPU: 
        model_parallel = models.ParallelModule(model=model)
        connect_parallel = models.ParallelModule(model=connect)
        if opt.load_model_path:
            model_parallel.load_model(opt.load_model_path)
        if opt.load_connect_path:
            connect_parallel.load_model(opt.load_connect_path)     
        model = model_parallel.model
        connect = connect_parallel.model       
    else:
        if opt.load_model_path:
            model.load_model(opt.load_model_path)
        if opt.load_connect_path:
            connect.load_model(opt.load_connect_path)
    model.cuda()
    connect.cuda()

    # step2: data
    log(log_file, 'Building dataset...')
    train_data = LocationDataset(data_root=opt.train_data_root, mask_size=opt.mask_size)
    val_data = LocationDataset(data_root=opt.val_data_root, mask_size=opt.mask_size)

    log(log_file, 'Building data loader...')
    train_dataloader = DataLoader(
        train_data, 
        opt.batch_size,
        shuffle=True,
        num_workers=opt.num_workers
    )
    val_dataloader = DataLoader(
        val_data, 
        opt.batch_size,
        shuffle=True,
        num_workers=opt.num_workers
    )
    
    # step3: criterion and optimizer
    log(log_file, 'Building criterion and optimizer...')
    lr = opt.lr_base
    optimizer = t.optim.Adam(
        list(model.parameters())+list(connect.parameters()), 
        lr = lr, 
        weight_decay=opt.weight_decay
    )
    current_epoch = opt.current_epoch
    # criterion = t.nn.CrossEntropyLoss()    
    weight = t.ones(utils.num_category+3)
    for i in range(utils.num_category+1):
        weight[i] = 1.25
    weight = weight.cuda()
    criterion = t.nn.CrossEntropyLoss(weight=weight)
    loss_meter = meter.AverageValueMeter() 
            
    # step4: training
    log(log_file, 'Starting to train...')
    if current_epoch == 0 and os.path.exists(opt.result_file):
        os.remove(opt.result_file)
    result_file = open(opt.result_file, 'a', newline='')
    writer = csv.writer(result_file)
    if current_epoch == 0:
        data_name = ['Epoch', 'Average Loss', 'Predict Accuracy', 'Number of Predict Categoey Right', \
            'Number of Target Categoey', 'Number of Predict Categoey', 'Category Accuracy', 'Category Proportion']
        writer.writerow(data_name)
        result_file.flush()

    while current_epoch < opt.max_epoch:
        current_epoch += 1
        running_loss = 0.0
        loss_meter.reset()
        log(log_file)
        log(log_file, f'Training epoch: {current_epoch}')

        for i, (input, target) in tqdm(enumerate(train_dataloader)):
            input = input.cuda()
            target = target.cuda()
            optimizer.zero_grad()
            score_model = model(input)
            score_connect = connect(score_model) 
            loss = criterion(score_connect, target)
            loss.backward()
            optimizer.step()       

            # log info
            running_loss += loss.item()
            if i % opt.print_freq == opt.print_freq - 1: 
                log(log_file, f'loss {running_loss / opt.print_freq:.5f}')
                running_loss = 0.0
            loss_meter.add(loss.item())
            
        if current_epoch % opt.save_freq == 0: 
            if opt.multi_GPU:
                model_parallel.save_model(current_epoch)
                connect_parallel.save_model(current_epoch)
            else:
                model.save_model(current_epoch)
                connect.save_model(current_epoch)
        average_loss = round(loss_meter.value()[0], 5)
        log(log_file, f'Average Loss: {average_loss}')

        # validate
        if current_epoch % opt.val_freq == 0: 
            predict_accuracy, num_predict_categoey_right, num_target_categoey, num_predict_categoey, \
                categoey_accuracy, categoey_proportion = val(model, connect, val_dataloader, log_file)
            results = [current_epoch, average_loss, predict_accuracy, num_predict_categoey_right, \
                num_target_categoey, num_predict_categoey, categoey_accuracy, categoey_proportion]
            writer.writerow(results)
            result_file.flush()

        # update learning rate
        if opt.update_lr:
            if current_epoch % opt.lr_decay_freq == 0:       
                lr = lr * (1 - float(current_epoch) / opt.max_epoch) ** 1.5
                for param_group in optimizer.param_groups:
                    param_group['lr'] = lr
                    log(log_file, f'Updating learning rate: {lr}')

    end_time = time.strftime("%b %d %Y %H:%M:%S")
    log(log_file, f'Training end time: {end_time}')
    log_file.close()
    result_file.close()

def val(model, connect, dataloader, file):
    model.eval()
    connect.eval()
    predict_accuracy = 0
    num_predict_categoey_right = 0
    num_target_categoey = 0
    num_predict_categoey = 0
    softmax = t.nn.Softmax(dim=1)

    for _, (input, target) in enumerate(dataloader):
        batch_size = input.shape[0]
        with t.no_grad():
            input = input.cuda()
            score_model = model(input)
            score_connect = connect(score_model)
    
        score_softmax = softmax(score_connect)
        output = score_softmax.cpu().numpy()
        predict = np.argmax(output, axis=1)
        target = target.numpy()
        for i in range(batch_size):
            num_predict = np.sum(predict[i] == target[i]) 
            predict_accuracy += (num_predict / (input.shape[2]*input.shape[3]))
            for k in range(utils.num_category+1):
                num_predict_categoey_right += np.sum((predict[i] == k) & (target[i] == k))
                num_target_categoey += np.sum(target[i] == k)
                num_predict_categoey += np.sum(predict[i] == k)  
        
    model.train()
    connect.train()
    predict_accuracy = round(predict_accuracy/len(dataloader.dataset), 5)
    num_predict_categoey_right = int(num_predict_categoey_right/len(dataloader.dataset))
    num_target_categoey = int(num_target_categoey/len(dataloader.dataset))
    num_predict_categoey = int(num_predict_categoey/len(dataloader.dataset))
    if num_target_categoey != 0:
        category_accuracy = round(num_predict_categoey_right / num_target_categoey, 5)
    else:
        category_accuracy = "nan"
    if num_predict_categoey != 0:
        category_proportion = round(num_predict_categoey_right / num_predict_categoey, 5)
    else:
        category_proportion = "nan"
    log(file, f'Predict Accuracy: {predict_accuracy}')
    log(file, f'Number of Predict Categoey Right: {num_predict_categoey_right}')
    log(file, f'Number of Target Categoey: {num_target_categoey}')
    log(file, f'Number of Predict Categoey: {num_predict_categoey}')
    log(file, f'Category Accuracy: {category_accuracy}')
    log(file, f'Category Proportion: {category_proportion}')
    return predict_accuracy, num_predict_categoey_right, num_target_categoey, num_predict_categoey, \
        category_accuracy, category_proportion

def help():
    print("""
    usage : python file.py <function> [--args=value]
    <function> := train | help
    example: 
            python {0} train --lr=0.01
            python {0} help
    avaiable args:""".format(__file__))

    source = (getsource(opt.__class__))
    print(source)

if __name__=='__main__':
    fire.Fire()