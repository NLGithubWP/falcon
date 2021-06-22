package master

import (
	"falcon_platform/cache"
	"falcon_platform/client"
	"falcon_platform/common"
	"falcon_platform/logger"
	"net/rpc"
	"time"
)

func RunMaster(masterAddr string, dslOjb *cache.DslObj, workerType string) (master *Master) {
	// launch 4 thread,
	// 1. heartbeat loop, stopped by master.Cancel()
	// 2. waiting for worker register, stopped by master.Cancel()
	// 3. rpc server, used to get requests from worker, stopped by master.StopRPCServer
	// 4. scheduling process, call finish to stop above threads

	master = newMaster(masterAddr, len(dslOjb.PartyAddrList))
	master.workerType = workerType
	master.reset()

	// thread 1, heartBeat
	go master.heartBeat()

	rpcServer := rpc.NewServer()
	logger.Log.Println("[master_rpc/RunMaster] rpcServer ready, now register with master")
	err := rpcServer.Register(master)
	// NOTE the rpc native Register() method will produce warning in console:
	// rpc.Register: method ...; needs exactly three
	// reply type of method ... is not a pointer: "bool"
	// all those can be ignored
	if err != nil {
		logger.Log.Println("rpcServer Register master Error", err)
		return
	}

	logger.Log.Println("[master_rpc/RunMaster] rpcServer registered with master")

	// thread 2
	go master.forwardRegistrations(dslOjb)

	// thread 3
	// launch a rpc server thread to process the requests.
	master.StartRPCServer(rpcServer, false)

	dispatcher := func() {
		master.dispatch(dslOjb)
	}

	var updateStatus func(jsonString string)
	var finish func()

	if workerType == common.TrainWorker {

		updateStatus = func(jsonString string) {
			// call coordinator to update status
			client.JobUpdateResInfo(common.CoordAddr, "", jsonString, "", dslOjb.JobId)
			master.jobStatusLock.Lock()
			jobStatus := master.jobStatus
			master.jobStatusLock.Unlock()
			client.JobUpdateStatus(common.CoordAddr, jobStatus, dslOjb.JobId)
			client.ModelUpdate(common.CoordAddr, 1, dslOjb.JobId)
		}

		finish = func() {
			// stop master after finishing the job
			master.StopRPCServer(master.Addr, "Master.Shutdown")
			// stop other related threads
			// close heartBeat and forwardRegistrations
			master.Clear()
		}

	} else if workerType == common.InferenceWorker {

		updateStatus = func(jsonString string) {
			client.InferenceUpdateStatus(common.CoordAddr, master.jobStatus, dslOjb.JobId)
		}

		finish = func() {
			// stop both master after finishing the job
			master.StopRPCServer(master.Addr, "Master.Shutdown")
			// stop other related threads
			// close heartBeat and forwardRegistrations
			master.Clear()
		}
	}

	// set time out, no worker comes within 1 min, stop master
	time.AfterFunc(1*time.Minute, func() {
		if len(master.workers) < master.workerNum {

			logger.Log.Printf("Master: Wait for 1 Min, No enough worker come, stop, required %d, got %d ",
				master.workerNum,
				len(master.workers),
			)

			finish()
		}
	})

	// thread 4
	// launch a thread to process then do the scheduling.
	go master.run(dispatcher, updateStatus, finish)

	return
}