package worker

import (
	"coordinator/common"
	"coordinator/distributed/base"
	"coordinator/distributed/entity"
	"coordinator/logger"
	"net/rpc"
	"time"
)

type TrainWorker struct {
	base.WorkerBase
}

func InitTrainWorker (masterAddr, workerAddr string) *TrainWorker{

	wk := TrainWorker{}
	wk.InitWorkerBase(workerAddr, common.TrainWorker)
	wk.MasterAddr = masterAddr

	return &wk
}


func (wk *TrainWorker) Run(){

	// 0 thread: start event Loop
	go wk.EventLoop()

	rpcSvc := rpc.NewServer()

	err := rpcSvc.Register(wk)
	if err!= nil{
		logger.Do.Fatalf("%s: start Error \n", wk.Name)
	}

	logger.Do.Printf("%s: register to masterAddr = %s \n", wk.Name, wk.MasterAddr)
	wk.Register(wk.MasterAddr)

	// start rpc server blocking...
	wk.StartRPCServer(rpcSvc, true)

}


func (wk *TrainWorker) DoTask (arg []byte, rep *entity.DoTaskReply) error {

	var dta *entity.DoTaskArgs = entity.DecodeDoTaskArgs(arg)

	//TestTaskProcess(dta)
	wk.TrainTask(dta, rep)


	for i := 10; i > 0; i-- {
		logger.Do.Println("Worker: Counting down before job done... ", i)
		time.Sleep(time.Second)
	}
	logger.Do.Printf("Worker: %s: task done\n", wk.Addr)
	return nil
}