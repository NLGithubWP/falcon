package master

import (
	"coordinator/cache"
	"coordinator/common"
	"coordinator/distributed/base"
	"coordinator/distributed/entity"
	"coordinator/distributed/utils"
	"coordinator/logger"
	"strings"
	"sync"
)

type Master struct {
	base.RpcBaseClass

	doneChannel chan bool

	beginCountDown *sync.Cond
	allWorkerReady *sync.Cond

	// tmp slice to store registered workers
	tmpWorkers chan string
	// slice to store valid workers
	workers   []string
	workerNum int

	jobStatus uint

	lastSendTime     int64
	heartbeatTimeout int

	foundWorker bool

	// common.TrainWorker or common.InferenceWorker
	workerType string
}

func newMaster(masterAddr string, workerNum int) (ms *Master) {
	ms = new(Master)
	ms.InitRpcBase(masterAddr)
	ms.Name = common.Master
	ms.beginCountDown = sync.NewCond(ms)
	ms.allWorkerReady = sync.NewCond(ms)

	ms.doneChannel = make(chan bool)
	ms.tmpWorkers = make(chan string)
	ms.workerNum = workerNum
	ms.heartbeatTimeout = common.MasterTimeout
	return
}

// Register is an RPC method that is called by workers after they have started
// up to report that they are ready to receive tasks.
func (this *Master) Register(args *entity.RegisterArgs, _ *struct{}) error {

	this.tmpWorkers <- args.WorkerAddr
	return nil
}

// sends information of worker to ch. which is used by scheduler
func (this *Master) forwardRegistrations(qItem *cache.QItem) {

	logger.Do.Printf("Master: start forwardRegistrations... ")
	var requiredIp []string

	for i := 0; i < len(qItem.AddrList); i++ {
		ip := strings.Split(qItem.AddrList[i], ":")[0]
		requiredIp = append(requiredIp, ip)
	}

loop:
	for {
		select {
		case <-this.Ctx.Done():
			logger.Do.Printf("Master: %s quit forwardRegistrations \n", this.Port)
			break loop

		case addr := <-this.tmpWorkers:
			// 1. check if this work already exist
			if utils.Contains(addr, this.workers) {
				logger.Do.Printf("Master: the worker %s already registered, skip \n", addr)
			}

			// 2. check if this worker is needed
			tmpIp := strings.Split(addr, ":")[0]

			for i, ip := range requiredIp {
				if tmpIp == ip {
					logger.Do.Println("Master: Found one worker", addr)

					this.Lock()
					this.workers = append(this.workers, addr)
					this.Unlock()
					this.beginCountDown.Broadcast()

					// remove the i th ip
					requiredIp = append(requiredIp[0:i+1], requiredIp[i+1:]...)
					break
				}
			}

			this.Lock()
			if len(this.workers) == this.workerNum {
				// it is not strictly necessary for you to hold the lock on M sync.Mutex when calling C.Broadcast()
				this.allWorkerReady.Broadcast()
			}
			this.Unlock()
		}
	}
}

func (this *Master) run(
	schedule func() string,
	updateStatus func(jsonString string),
	finish func(),
) {

	jsonString := schedule()
	logger.Do.Println("Master: finish job, begin to update to coord")

	updateStatus(jsonString)
	logger.Do.Println("Master: finish job, begin to close all")

	finish()
	logger.Do.Printf("Master %s: job completed\n", this.Addr)

	this.doneChannel <- true
}

func (this *Master) Wait() {

loop:
	for {
		select {
		case <-this.Ctx.Done():
			logger.Do.Printf("WorkerBase: server %s quit Waitting \n", this.Addr)
			break loop
		case <-this.doneChannel:
			break loop
		}
	}
}

// Shutdown is an RPC method that shuts down the Master's RPC server.
// for rpc method, must be public method, only 2 params, second one must be pointer,return err type
func (this *Master) Shutdown(_, _ *struct{}) error {
	logger.Do.Println("Master: Shutdown server")
	_ = this.Listener.Close() // causes the Accept to fail, then break out the accetp loop
	return nil
}

func (this *Master) killWorkers() {

	this.Lock()
	defer this.Unlock()

	for _, worker := range this.workers {

		this.StopRPCServer(worker, this.workerType+".Shutdown")
	}
}

func (this *Master) KillJob(_, _ *struct{}) error {
	/**
	 * @Author
	 * @Description called by coordinator, to shutdown the running job
	 * @Date 9:47 上午 14/12/20
	 * @Param
	 * @return
	 **/
	this.killWorkers()

	this.Cancel()
	this.StopRPCServer(this.Addr, "Master.Shutdown")
	return nil
}
