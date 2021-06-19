package router

import (
	"bytes"
	"encoding/json"
	"falcon_platform/client"
	"falcon_platform/common"
	"falcon_platform/coordserver/controller"
	"falcon_platform/coordserver/entity"
	"falcon_platform/exceptions"
	"falcon_platform/logger"
	"fmt"
	"net/http"
	"strconv"

	"github.com/gorilla/mux"
)

type JobSubmitRes struct {
	JobId    uint   `json:"job_id"`
	JobName  string `json:"job_name"`
	UserId   uint   `json:"user_id"`
	PartyIds string `json:"party_ids"`
	TaskNum  uint   `json:"task_num,uint"`
	Status   string `json:"status"`
}

type JobStatusRes struct {
	JobId  uint   `json:"job_id"`
	Status string `json:"status"`
}

type JobIdGet struct {
	JobId string `json:"job_id"`
}

// receive a job info file, parse it, put in dslqueue
func SubmitTrainJobFile(w http.ResponseWriter, r *http.Request, ctx *entity.Context) {
	// Parse multipart form, 32 << 20 specifies a maximum
	// upload of 32 MB files.
	_ = r.ParseMultipartForm(32 << 20)

	// 1. read file from multiform
	var buf bytes.Buffer
	err, contents := client.ReceiveFile(r, buf, common.TrainJobFileKey)
	if err != nil {
		errMsg := fmt.Sprintf("client.ReceiveFile Error %s", err)
		exceptions.HandleHttpError(w, r, http.StatusBadRequest, errMsg)
		return
	}
	logger.Log.Println("client.ReceiveFile success")

	// 2. parser to object
	var job common.TrainJob
	err = common.ParseTrainJob(contents, &job)
	if err != nil {
		errMsg := fmt.Sprintf("common.ParseJob Error %s", err)
		exceptions.HandleHttpError(w, r, http.StatusBadRequest, errMsg)
		return
	}
	logger.Log.Println("common.ParseJob success")

	// 3. submit job with parsed object
	JobId, JobName, UserId, PartyIds, TaskNum, Status := controller.JobSubmit(&job, ctx)

	// 4. return to client
	buf.Reset()

	resIns := JobSubmitRes{
		JobId,
		JobName,
		UserId,
		PartyIds,
		TaskNum,
		Status}

	err = json.NewEncoder(w).Encode(resIns)

	if err != nil {
		errMsg := fmt.Sprintf("JSON Marshal Error %s", err)
		exceptions.HandleHttpError(w, r, http.StatusInternalServerError, errMsg)
		return
	}
}

func JobKill(w http.ResponseWriter, r *http.Request, ctx *entity.Context) {
	// read the query parameters with gorilla mux
	params := mux.Vars(r)
	jobId, _ := strconv.Atoi(params["jobId"])
	var jr JobStatusRes
	status, e := controller.JobStatusQuery(uint(jobId), ctx)
	if e != nil {
		exceptions.HandleHttpError(w, r, http.StatusBadRequest, e.Error())
		return
	}
	if status == common.JobFailed || status == common.JobKilled || status == common.JobSuccessful {
		jr = JobStatusRes{
			JobId:  uint(jobId),
			Status: "Job already stopped with status: " + status,
		}
	} else {
		err := controller.JobKill(uint(jobId), ctx)
		if err != nil {
			exceptions.HandleHttpError(w, r, http.StatusBadRequest, err.Error())
			return
		}
		jr = JobStatusRes{
			JobId:  uint(jobId),
			Status: common.JobKilled,
		}
	}

	err := json.NewEncoder(w).Encode(jr)

	if err != nil {
		errMsg := fmt.Sprintf("JSON Marshal Error %s", err)
		exceptions.HandleHttpError(w, r, http.StatusInternalServerError, errMsg)
		return
	}

}

func JobUpdateMaster(w http.ResponseWriter, r *http.Request, ctx *entity.Context) {

	client.ReceiveForm(r)

	JobId := r.FormValue(common.JobId)
	MasterAddr := r.FormValue(common.MasterAddrKey)

	jobId, _ := strconv.Atoi(JobId)

	controller.JobUpdateMaster(uint(jobId), MasterAddr, ctx)

}

func JobUpdateStatus(w http.ResponseWriter, r *http.Request, ctx *entity.Context) {
	client.ReceiveForm(r)
	JobId := r.FormValue(common.JobId)
	JobStatus := r.FormValue(common.JobStatus)

	jobId, e := strconv.Atoi(JobId)
	if e != nil {
		panic(e)
	}

	controller.JobUpdateStatus(uint(jobId), JobStatus, ctx)
}

func JobUpdateResInfo(w http.ResponseWriter, r *http.Request, ctx *entity.Context) {
	client.ReceiveForm(r)
	JobId := r.FormValue(common.JobId)
	JobErrMsg := r.FormValue(common.JobErrMsg)
	JobResult := r.FormValue(common.JobResult)
	JobExtInfo := r.FormValue(common.JobExtInfo)

	jobId, _ := strconv.Atoi(JobId)

	controller.JobUpdateResInfo(uint(jobId), JobErrMsg, JobResult, JobExtInfo, ctx)
}

func JobStatusQuery(w http.ResponseWriter, r *http.Request, ctx *entity.Context) {

	// read the query parameters with gorilla mux
	params := mux.Vars(r)
	jobId, _ := strconv.Atoi(params["jobId"])

	status, err := controller.JobStatusQuery(uint(jobId), ctx)

	if err != nil {
		exceptions.HandleHttpError(w, r, http.StatusBadRequest, err.Error())
		return
	}

	jr := JobStatusRes{
		JobId:  uint(jobId),
		Status: status,
	}

	err = json.NewEncoder(w).Encode(jr)

	if err != nil {
		errMsg := fmt.Sprintf("JSON Marshal Error %s", err)
		exceptions.HandleHttpError(w, r, http.StatusInternalServerError, errMsg)
		return
	}

}
