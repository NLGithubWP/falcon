package controller

import "falcon_platform/coordserver/entity"

func ModelUpdate(jobId, isTrained uint, ctx *entity.Context) uint {
	tx := ctx.JobDB.DB.Begin()
	e, u := ctx.JobDB.ModelUpdate(tx, jobId, isTrained)
	ctx.JobDB.Commit(tx, e)
	return u.ID

}
