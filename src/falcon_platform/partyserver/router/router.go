package router

import (
	"encoding/json"
	"falcon_platform/client"
	"falcon_platform/common"
	"falcon_platform/exceptions"
	"falcon_platform/jobmanager/comms_pattern"
	"falcon_platform/logger"
	"falcon_platform/partyserver/controller"
	"fmt"
	"github.com/gorilla/mux"
	"net/http"
	"strconv"
)

func NewRouter() *mux.Router {
	r := mux.NewRouter()

	// sanity check
	r.HandleFunc("/", HelloPartyServer).Methods("GET")

	r.HandleFunc(common.RunWorker, RunWorker()).Methods("POST")

	return r
}

// RunWorker
// * @Description Called by job manager to launch one or multiple workers
// * @Date 下午2:51 23/08/21
// * @Param
// * @return  Return job manager with resources' information, eg. address etc, defined in common.PartyRunWorkerReply
func RunWorker() func(w http.ResponseWriter, r *http.Request) {
	return func(w http.ResponseWriter, r *http.Request) {

		logger.Log.Println("[PartyServer]: RunWorker registering partyServerPort to coord", common.PartyServerPort)

		// TODO: why is this via Form, and not via JSON?
		// both Form and Json are ok.  Json may be more friendly.
		client.ReceiveForm(r)

		// this is sent from main http server
		masterAddr := r.FormValue(common.MasterAddrKey)
		workerTypeKey := r.FormValue(common.TaskTypeKey)
		jobId := r.FormValue(common.JobId)
		// task input path, model path, output path,
		dataPath := r.FormValue(common.TrainDataPath)
		modelPath := r.FormValue(common.ModelPath)
		dataOutput := r.FormValue(common.TrainDataOutput)
		// numbers
		WorkerPreParty, err := strconv.Atoi(r.FormValue(common.WorkerPreParty))
		partyNum, err := strconv.Atoi(r.FormValue(common.TotalPartyNumber))
		stageName := r.FormValue(common.StageClassIDKey)
		jobType := r.FormValue(common.JobTypeKey)

		if err != nil {
			panic(err)
		}

		// retrieve global cfg for this job.
		jobNetCfg := comms_pattern.GetAllNetworkCfg()[jobType]

		resIns := controller.RunWorker(masterAddr, workerTypeKey, jobId, dataPath, modelPath, dataOutput, WorkerPreParty, partyNum, stageName, jobNetCfg)

		// return to job manager
		w.WriteHeader(http.StatusOK)

		reply := comms_pattern.RunWorkerReply{
			EncodedStr: comms_pattern.EncodePartyRunWorkerReply(resIns),
		}

		err = json.NewEncoder(w).Encode(reply)

		if err != nil {
			errMsg := fmt.Sprintf("JSON Marshal Error %s", err)
			exceptions.HandleHttpError(w, r, http.StatusInternalServerError, errMsg)
			return
		}
	}
}

// HelloPartyServer sanity check
func HelloPartyServer(w http.ResponseWriter, req *http.Request) {
	w.WriteHeader(http.StatusOK)
	_, _ = fmt.Fprintf(w, "hello from falcon party server~\n")
}
