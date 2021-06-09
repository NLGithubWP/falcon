package controller

import (
	"encoding/json"
	"falcon_platform/cache"
	"falcon_platform/common"
	"falcon_platform/coordserver/entity"
	"falcon_platform/distributed"
	"falcon_platform/logger"
)

func JobSubmit(job *common.TrainJob, ctx *entity.Context) (
	uint, // JobId
	string, // JobName
	uint, // UserID
	string, // PartyIds
	uint, // TaskNum
	string, // Status
) {

	logger.Log.Println("HTTP server: in SubmitJob, put to the JobQueue")

	// generate.sh item pushed to the queue

	addresses := common.ParseAddress(job.PartyInfo)

	// generate.sh strings used to write to db
	partyIds, err := json.Marshal(job.PartyInfo)
	TaskInfo, err := json.Marshal(job.Tasks)
	if err != nil {
		panic("json.Marshal(job.PartyIds) error")
	}

	ModelName := job.Tasks.ModelTraining.AlgorithmName
	ModelInfo := job.Tasks.ModelTraining.AlgorithmName
	PartyNumber := uint(len(job.PartyInfo))

	// write to db
	tx := ctx.JobDB.DB.Begin()
	err1, u1 := ctx.JobDB.JobInfoCreate(
		tx,
		job.JobName,
		ctx.UsrId,
		string(partyIds),
		string(TaskInfo),
		job.JobInfo,
		PartyNumber,
		job.JobFlType,
		job.ExistingKey,
		job.TaskNum)

	err2, u2 := ctx.JobDB.JobSubmit(tx, ctx.UsrId, common.JobInit, u1.Id)
	err3, _ := ctx.JobDB.SvcCreate(tx, u2.JobId)

	err4, _ := ctx.JobDB.ModelCreate(tx, u2.JobId, ModelName, ModelInfo)

	ctx.JobDB.Commit(tx, []error{err1, err2, err3, err4})

	qItem := new(cache.QItem)
	qItem.AddrList = addresses
	qItem.JobId = u2.JobId
	qItem.JobName = job.JobName
	qItem.JobFlType = job.JobFlType
	qItem.ExistingKey = job.ExistingKey
	qItem.PartyNums = job.PartyNums
	qItem.PartyInfo = job.PartyInfo
	qItem.Tasks = job.Tasks

	go func() {
		cache.JobQueue.Push(qItem)
	}()

	return u2.JobId, u1.JobName, u2.UserID, u1.PartyIds, u1.TaskNum, u2.Status
}

func JobKill(jobId uint, ctx *entity.Context) error {

	e, u := ctx.JobDB.JobGetByJobID(jobId)
	if e != nil {
		return e
	}

	distributed.KillJob(u.MasterAddr, common.Network)

	tx := ctx.JobDB.DB.Begin()
	err, _ := ctx.JobDB.JobUpdateStatus(tx, jobId, common.JobKilled)
	ctx.JobDB.Commit(tx, err)
	return err
}

func JobUpdateMaster(jobId uint, masterAddr string, ctx *entity.Context) {
	tx := ctx.JobDB.DB.Begin()
	e, _ := ctx.JobDB.SvcUpdateMaster(tx, jobId, masterAddr)
	e2, _ := ctx.JobDB.JobUpdateMaster(tx, jobId, masterAddr)
	ctx.JobDB.Commit(tx, []error{e, e2})
}

func JobUpdateResInfo(jobId uint, jobErrMsg, jobResult, jobExtInfo string, ctx *entity.Context) {
	tx := ctx.JobDB.DB.Begin()
	e, _ := ctx.JobDB.JobUpdateResInfo(tx, jobId, jobErrMsg, jobResult, jobExtInfo)
	ctx.JobDB.Commit(tx, e)
}

func JobUpdateStatus(jobId uint, status string, ctx *entity.Context) {
	tx := ctx.JobDB.DB.Begin()
	e, _ := ctx.JobDB.JobUpdateStatus(tx, jobId, status)
	ctx.JobDB.Commit(tx, e)
}

func JobStatusQuery(jobId uint, ctx *entity.Context) (string, error) {
	e, u := ctx.JobDB.JobGetByJobID(jobId)

	return u.Status, e
}