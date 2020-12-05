package controller

import (
	"coordinator/api/entity"
	"coordinator/cache"
	"coordinator/common"
	dist "coordinator/distributed"
	"encoding/json"
)



func CreateService(jobId uint, appName, extInfo string, ctx *entity.Context) (uint,string){

	ctx.Ms.Tx = ctx.Ms.Db.Begin()

	e1, u1 := ctx.Ms.ModelGetByID(jobId)

	e2, u := ctx.Ms.CreateService(appName, u1.ID, jobId, extInfo)

	e3, u2 := ctx.Ms.JobGetByJobID(jobId)

	ctx.Ms.Commit([]error{e1, e2, e3})


	var pInfo []common.PartyInfo
	var taskInfos common.Tasks

	err := json.Unmarshal([]byte(u2.PartyIds), &pInfo)
	err2 := json.Unmarshal([]byte(u2.TaskInfos), &taskInfos)
	if err != nil || err2!=nil {
		panic("json.Unmarshal(PartyIds or TaskInfos) error")
	}

	iPs ,partyPath, modelPath, executablePath := common.ParsePartyInfo(pInfo, taskInfos)

	qItem := new(cache.QItem)
	qItem.IPs = iPs
	qItem.JobId = jobId
	qItem.PartyPath = partyPath
	qItem.TaskInfos = taskInfos
	qItem.ModelPath = modelPath
	qItem.ExecutablePath = executablePath

	go dist.SetupDist(qItem, common.PredictExecutor)

	return u.ID, u.ModelServiceName
}


func UpdateService(dsl *common.DSL, ctx *entity.Context) {

}


func QueryService(dsl *common.DSL, ctx *entity.Context) {

}


func StopService(dsl *common.DSL, ctx *entity.Context) {

}


func DeleteService(dsl *common.DSL, ctx *entity.Context) {

}


func PublishService(jobId uint, isPublish uint, ctx *entity.Context) {
	ctx.Ms.Tx = ctx.Ms.Db.Begin()

	e, _ := ctx.Ms.PublishService(jobId, isPublish)
	ctx.Ms.Commit(e)
}


func LaunchService(dsl *common.DSL, ctx *entity.Context) {
}


func ModelServiceUpdateStatus(jobId uint, status uint, ctx *entity.Context){
	ctx.Ms.Tx = ctx.Ms.Db.Begin()
	e, _ := ctx.Ms.ModelServiceUpdateStatus(jobId, status)
	ctx.Ms.Commit(e)

}