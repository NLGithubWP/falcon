package distributed

import (
	"coordinator/cache"
	c "coordinator/client"
	"coordinator/common"
	"coordinator/distributed/taskmanager"
	"coordinator/logger"
	"fmt"
	"strings"
)

func SetupDistProd(qItem *cache.QItem, workerType string) {
	/**
	 * @Author
	 * @Description run master, and then, master will call lister to run worker
	 * @Date 2:36 下午 5/12/20
	 * @Param
	 * @return
	 **/

	masterPort := c.GetFreePort(common.CoordAddr)
	logger.Do.Println("SetupDist: Launch master Get port", masterPort)

	masterIp := common.CoordIP
	masterAddr := masterIp + ":" + masterPort

	logger.Do.Println("SetupDist: Launch master ProdEnv")

	// in prod, use k8s to run train/predict server as a isolate process
	itemKey := "job" + fmt.Sprintf("%d", qItem.JobId)

	serviceName := "master-" + itemKey + "-" + strings.ToLower(workerType)

	// put to the queue, assign key to env
	logger.Do.Println("SetupDist: Writing item to redis")

	cache.InitRedisClient().Set(itemKey, cache.Serialize(qItem))

	logger.Do.Printf("SetupDist: Get key, %s InitK8sManager\n", itemKey)

	km := taskmanager.InitK8sManager(true, "")

	command := []string{
		common.MasterYamlCreatePath,
		serviceName,
		masterPort,
		itemKey,
		workerType,
		masterAddr,
		common.Master,
		common.CoordK8sSvcName,
		common.Env,
	}

	//_=taskmanager.ExecuteOthers("ls")
	//_=taskmanager.ExecuteOthers("pwd")
	km.UpdateYaml(strings.Join(command, " "))

	logger.Do.Println("SetupDist: Creating yaml done")

	filename := common.YamlBasePath + serviceName + ".yaml"

	logger.Do.Println("SetupDist: Creating Resources based on file, ", filename)

	km.CreateResources(filename)
	logger.Do.Println("SetupDist: setup master done")
}

func SetupWorkerHelperProd(masterAddr, workerType, jobId, dataPath, modelPath, dataOutput string) {

	/**
	 * @Author
	 * @Description: this func is only called by partyserver
	 * @Date 2:14 下午 1/12/20
	 * @Param
	 	httpHost： 		IP of the partyserver addr
		masterAddr： IP of the master addr
		masterAddr： train or predictor
	 **/
	logger.Do.Println("SetupWorkerHelper: Creating parameters:", masterAddr, workerType)

	workerPort := c.GetFreePort(common.CoordAddr)

	workerAddr := common.PartyServerIP + ":" + workerPort
	var serviceName string

	// in dev, use thread

	if workerType == common.TrainWorker {

		serviceName = "worker-job" + jobId + "-train-" + common.PartyServerId

		logger.Do.Println("SetupWorkerHelper: Current in Prod, TrainWorker, svcName", serviceName)
	} else if workerType == common.InferenceWorker {

		serviceName = "worker-job" + jobId + "-predict-" + common.PartyServerId

		logger.Do.Println("SetupWorkerHelper: Current in Prod, InferenceWorker, svcName", serviceName)
	}

	km := taskmanager.InitK8sManager(true, "")
	command := []string{
		common.WorkerYamlCreatePath,
		serviceName,               // 1. worker service name
		workerPort,                // 2. worker service port
		masterAddr,                // 3. master addr
		workerType,                // 4. train or predict job
		workerAddr,                // 5. worker addr
		workerType,                // 6. serviceName train or predict
		common.Env,                // 7. env or prod
		common.PartyServeBasePath, // 8. folder to store logs, the same as partyserver folder currently,
		dataPath,                  // 9. folder to read train data
		modelPath,                 // 10. folder to store models
		dataOutput,                // 11. folder to store processed data
	}

	_ = taskmanager.ExecuteOthers("ls")
	_ = taskmanager.ExecuteOthers("pwd")
	km.UpdateYaml(strings.Join(command, " "))

	filename := common.YamlBasePath + serviceName + ".yaml"

	logger.Do.Println("SetupDist: Creating yaml done", filename)

	km.CreateResources(filename)

	logger.Do.Println("SetupDist: worker is running")

}
