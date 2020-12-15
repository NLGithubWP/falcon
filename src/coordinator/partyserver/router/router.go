package router

import (
	"coordinator/client"
	"coordinator/common"
	"coordinator/partyserver/controller"
	"coordinator/logger"
	"net/http"
)

func SetupWorker() func(w http.ResponseWriter, r *http.Request) {
	return func(w http.ResponseWriter, r *http.Request) {

		client.ReceiveForm(r)

		// this is sent from main http server
		masterAddress := r.FormValue(common.MasterAddr)
		taskType := r.FormValue(common.TaskType)
		jobId := r.FormValue(common.JobId)
		dataPath := r.FormValue(common.TrainDataPath)
		modelPath := r.FormValue(common.ModelPath)
		dataOutput := r.FormValue(common.TrainDataOutput)

		go func(){
			defer logger.HandleErrors()
			controller.SetupWorker(masterAddress, taskType, jobId, dataPath,modelPath, dataOutput)
		}()

	}
}
