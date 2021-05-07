package main

import (
	"coordinator/cache"
	"coordinator/common"
	"coordinator/coordserver"
	"coordinator/distributed"
	"coordinator/distributed/taskmanager"
	"coordinator/distributed/worker"
	"coordinator/logger"
	"coordinator/partyserver"
	"fmt"
	"os"
	"path"
	"runtime"
	"strconv"
	"time"
)

func init() {

	// prority: env >  user provided > default value
	runtime.GOMAXPROCS(4)
	// before init the envs, load the meta envs
	// use os to get the env, since initLogger is before initEnv
	common.Env = os.Getenv("ENV")
	common.ServiceName = os.Getenv("SERVICE_NAME")
	common.LogPath = os.Getenv("LOG_PATH")

	initLogger()
	initEnv(common.ServiceName)

}

func initLogger() {
	var runtimeLogPath string

	// in dev, we have a logPath to store everything,
	// but in production, the coordinator and part server are
	// separated at different machines or clusters, we use docker,
	if common.Env == common.DevEnv {
		runtimeLogPath = path.Join(common.LogPath, common.RuntimeLogs)
	} else if common.Env == common.ProdEnv {
		// the log is fixed to ./log, which is the path inside the docker
		runtimeLogPath = "./logs"
	}

	fmt.Println("common.RuntimeLogs at: ", runtimeLogPath)

	_ = os.Mkdir(runtimeLogPath, os.ModePerm)
	// Use layout string for time format.
	const layout = "2006-01-02T15:04:05"
	// Place now in the string.
	rawTime := time.Now()

	var logFileName string
	logFileName = runtimeLogPath + "/" + common.ServiceName + "-" + rawTime.Format(layout) + ".log"

	logger.Do, logger.F = logger.GetLogger(logFileName)
}

func initEnv(svcName string) {

	if svcName == "coord" {
		// find the cluster port, call internally
		common.CoordIP = common.GetEnv("COORD_SERVER_IP", "")
		common.CoordPort = common.GetEnv("COORD_SERVER_PORT", "30004")

		common.CoordAddr = (common.CoordIP + ":" + common.CoordPort)

		common.CoordBasePath = common.GetEnv("COORD_SERVER_BASEPATH", "./dev_test")

		// coord http server number of consumers
		common.NbConsumers = common.GetEnv("N_CONSUMER", "3")

		// get the env for Job DB in Coord server
		common.JobDatabase = common.GetEnv("JOB_DATABASE", "sqlite3")

		// get the env needed for different db type
		if common.JobDatabase == common.DBsqlite3 {
			common.JobDbSqliteDb = common.GetEnv("JOB_DB_SQLITE_DB", "falcon.db")
		} else if common.JobDatabase == common.DBMySQL {
			common.JobDbHost = common.GetEnv("JOB_DB_HOST", "localhost")
			common.JobDbMysqlUser = common.GetEnv("JOB_DB_MYSQL_USER", "falcon")
			common.JobDbMysqlPwd = common.GetEnv("JOB_DB_MYSQL_PWD", "falcon")
			common.JobDbMysqlDb = common.GetEnv("JOB_DB_MYSQL_DB", "falcon")
			common.JobDbMysqlOptions = common.GetEnv("JOB_DB_MYSQL_OPTIONS", "?parseTime=true")
			common.JobDbMysqlPort = common.GetEnv("MYSQL_CLUSTER_PORT", "30000")
		}

		// env for prod
		if common.Env == common.ProdEnv {

			common.RedisHost = common.GetEnv("REDIS_HOST", "localhost")
			common.RedisPwd = common.GetEnv("REDIS_PWD", "falcon")
			// coord needs redis information
			common.RedisPort = common.GetEnv("REDIS_CLUSTER_PORT", "30002")
			// find the cluster port, call internally
			common.JobDbMysqlNodePort = common.GetEnv("MYSQL_NODE_PORT", "30001")
			common.RedisNodePort = common.GetEnv("REDIS_NODE_PORT", "30003")

			common.CoordK8sSvcName = common.GetEnv("COORD_SVC_NAME", "")
		}

		if len(common.ServiceName) == 0 {
			logger.Do.Println("Error: Input Error, ServiceName not provided, is either 'coord' or 'partyserver' ")
			os.Exit(1)
		}

	} else if svcName == "partyserver" {

		// partyserver needs coord IP+port,lis port
		common.CoordIP = common.GetEnv("COORD_SERVER_IP", "")
		common.CoordPort = common.GetEnv("COORD_SERVER_PORT", "30004")
		common.PartyServerIP = common.GetEnv("PARTY_SERVER_IP", "")

		// partyserver communicate coord with IP+port
		common.CoordAddr = (common.CoordIP + ":" + common.CoordPort)

		// run partyserver requires to get a new partyserver port
		common.PartyServerPort = common.GetEnv("PARTY_SERVER_NODE_PORT", "")

		common.PartyID = common.GetEnv("PARTY_ID", "")

		common.PartyServerBasePath = common.GetEnv("PARTY_SERVER_BASEPATH", "./dev_test")

		// get the MPC exe path
		common.MpcExePath = common.GetEnv(
			"MPC_EXE_PATH",
			"/opt/falcon/third_party/MP-SPDZ/semi-party.x")
		// get the FL engine exe path
		common.FLEnginePath = common.GetEnv(
			"FL_ENGINE_PATH",
			"/opt/falcon/build/src/executor/falcon")
		if common.CoordIP == "" || common.PartyServerIP == "" || common.PartyServerPort == "" {
			logger.Do.Println("Error: Input Error, either CoordIP or PartyServerIP or PartyServerPort not provided")
			os.Exit(1)
		}

	} else if svcName == common.Master {

		common.CoordPort = common.GetEnv("COORD_SERVER_PORT", "30004")

		// master needs queue item, task type
		common.MasterQItem = common.GetEnv("ITEM_KEY", "")
		common.WorkerType = common.GetEnv("EXECUTOR_TYPE", "")
		common.MasterAddr = common.GetEnv("MASTER_ADDR", "")

		if common.Env == common.DevEnv {

			// master communicate coord with IP+port in dev
			logger.Do.Println("CoordIP: ", common.CoordIP+":"+common.CoordPort)

			common.CoordAddr = (common.CoordIP + ":" + common.CoordPort)

		} else if common.Env == common.ProdEnv {

			// master needs redis information
			common.RedisHost = common.GetEnv("REDIS_HOST", "localhost")
			common.RedisPwd = common.GetEnv("REDIS_PWD", "falcon")
			common.RedisPort = common.GetEnv("REDIS_CLUSTER_PORT", "30002")
			common.RedisNodePort = common.GetEnv("REDIS_NODE_PORT", "30003")

			// prod using k8
			common.CoordK8sSvcName = common.GetEnv("COORD_SVC_NAME", "")
			common.WorkerK8sSvcName = common.GetEnv("EXECUTOR_NAME", "")

			// master communicate coord with IP+port in dev, with name+port in prod
			logger.Do.Println("CoordK8sSvcName: ", common.CoordK8sSvcName+":"+common.CoordPort)

			common.CoordAddr = (common.CoordK8sSvcName + ":" + common.CoordPort)
		}

		if common.CoordAddr == "" {
			logger.Do.Println("Error: Input Error, CoordAddr not provided")
			os.Exit(1)
		}

	} else if svcName == common.TrainWorker {
		// this will be executed only in production, in dev, the common.WorkerType==""

		common.TaskDataPath = common.GetEnv("TASK_DATA_PATH", "")
		common.TaskModelPath = common.GetEnv("TASK_MODEL_PATH", "")
		common.TaskDataOutput = common.GetEnv("TASK_DATA_OUTPUT", "")
		common.TaskRuntimeLogs = common.GetEnv("RUN_TIME_LOGS", "")

		common.WorkerType = common.GetEnv("EXECUTOR_TYPE", "")
		common.WorkerAddr = common.GetEnv("WORKER_ADDR", "")
		common.MasterAddr = common.GetEnv("MASTER_ADDR", "")
		common.WorkerK8sSvcName = common.GetEnv("EXECUTOR_NAME", "")
		if common.MasterAddr == "" || common.WorkerAddr == "" {
			logger.Do.Println("Error: Input Error, either MasterAddr or WorkerAddr  not provided")
			os.Exit(1)
		}

	} else if svcName == common.InferenceWorker {

		common.TaskDataPath = common.GetEnv("TASK_DATA_PATH", "")
		common.TaskModelPath = common.GetEnv("TASK_MODEL_PATH", "")
		common.TaskDataOutput = common.GetEnv("TASK_DATA_OUTPUT", "")
		common.TaskRuntimeLogs = common.GetEnv("RUN_TIME_LOGS", "")

		// this will be executed only in production, in dev, the common.WorkerType==""

		common.WorkerType = common.GetEnv("EXECUTOR_TYPE", "")
		common.WorkerAddr = common.GetEnv("WORKER_ADDR", "")
		common.MasterAddr = common.GetEnv("MASTER_ADDR", "")
		common.WorkerK8sSvcName = common.GetEnv("EXECUTOR_NAME", "")
		if common.MasterAddr == "" || common.WorkerAddr == "" {
			logger.Do.Println("Error: Input Error, either MasterAddr or WorkerAddr not provided")
			os.Exit(1)
		}
	}
}

func main() {

	defer logger.HandleErrors()

	defer func() {
		_ = logger.F.Close()
	}()

	if common.ServiceName == "coord" {
		logger.Do.Println("Launch falcon_platform, the common.ServiceName", common.ServiceName)

		nConsumer, _ := strconv.Atoi(common.NbConsumers)
		coordserver.SetupHttp(nConsumer)
	}

	// start work in remote machine automatically
	if common.ServiceName == "partyserver" {

		logger.Do.Println("Launch falcon_platform, the common.ServiceName", common.ServiceName)

		partyserver.SetupPartyServer()
	}

	//////////////////////////////////////////////////////////////////////////
	//						 start tasks, called internally 				//
	// 																	    //
	//////////////////////////////////////////////////////////////////////////

	//those 2 is only called internally

	if common.ServiceName == common.Master {

		logger.Do.Println("Launching falcon_platform, the common.WorkerType", common.WorkerType)

		// this should be the service name, defined at runtime,
		masterAddr := common.MasterAddr

		qItem := cache.Deserialize(cache.InitRedisClient().Get(common.MasterQItem))

		workerType := common.WorkerType

		distributed.SetupMaster(masterAddr, qItem, workerType)

		km := taskmanager.InitK8sManager(true, "")
		km.DeleteService(common.WorkerK8sSvcName)
	}

	if common.ServiceName == common.TrainWorker {

		logger.Do.Println("Launching falcon_platform, the common.WorkerType", common.WorkerType)

		// init the train worker with addresses of master and worker, also the partyID
		wk := worker.InitTrainWorker(common.MasterAddr, common.WorkerAddr, common.PartyID)
		wk.RunWorker(wk)

		// once  worker is killed, clear the resources.
		km := taskmanager.InitK8sManager(true, "")
		km.DeleteService(common.WorkerK8sSvcName)

	}

	if common.ServiceName == common.InferenceWorker {

		logger.Do.Println("Launching falcon_platform, the common.WorkerType", common.WorkerType)

		wk := worker.InitInferenceWorker(common.MasterAddr, common.WorkerAddr, common.PartyID)
		wk.RunWorker(wk)
	}
	// once  worker is killed, clear the resources.
	km := taskmanager.InitK8sManager(true, "")
	km.DeleteService(common.WorkerK8sSvcName)
}
