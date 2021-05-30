package common

import (
	"falcon_platform/logger"
	"os"
)

const (
	// TODO: are these the names assigned for master and workers?
	Master          = "Master"
	TrainWorker     = "TrainWorker"
	InferenceWorker = "InferenceWorker"

	// master& worker heartbeat

	MasterTimeout = 10000 //  send heartbeat every 10 second
	WorkerTimeout = 20000 //  receive heartbeat within every 20 second

	// router path for coordServer
	UploadTrainJobFile   = "/api/upload-train-job-file"
	StopTrainJob         = "/api/stop-train-job"
	UpdateTrainJobMaster = "/api/update-train-master"
	UpdateJobResInfo     = "/api/update-job-res"
	UpdateJobStatus      = "/api/update-job-status"
	QueryTrainJobStatus  = "/api/query-train-job-status"

	UpdateInferenceJobMaster = "/api/update-inference-master"
	InferenceStatusUpdate    = "/api/update-prediction-service-status"
	InferenceUpdate          = "/api/prediction-service-update"
	InferenceCreate          = "/api/prediction-service-create"
	ModelUpdate              = "/api/model-update"

	AssignPort         = "/api/port-assign"
	AddPort            = "/api/portadd"
	GetPartyServerPort = "/api/port-get"

	PartyServerAdd    = "/api/party-server-add"
	PartyServerDelete = "/api/party-server-del"

	// router path for partyserver
	SetupWorker = "/api/setup-worker"

	// shared key of map
	PartyServerAddrKey = "psAddr"
	MasterAddrKey      = "masterAddr"
	PartyServerPortKey = "psPort"

	JobId      = "job_id"
	JobErrMsg  = "error_msg"
	JobResult  = "job_result"
	JobExtInfo = "ext_info"

	JobStatus        = "status"
	SubProcessNormal = "ok"

	TrainJobFileKey = "upload-train-job-file"

	TaskTypeKey = "task-type"

	TrainDataPath   = "train-data-path"
	TrainDataOutput = "train-data-output"
	ModelPath       = "model-path"
	IsTrained       = "is_trained"

	JobName = "job_name"
	ExtInfo = "ext_info"

	Network = "tcp"

	// job status
	// TODO: replace int with msgs
	JobInit       = 0
	JobRunning    = 1
	JobSuccessful = 2
	JobFailed     = 3
	JobKilled     = 4

	// for common.Env
	DevEnv  = "dev"
	ProdEnv = "prod"

	// for DB engine names
	DBsqlite3 = "sqlite3"
	DBMySQL   = "mysql"

	WorkerYamlCreatePath = "./scripts/_create_runtime_worker.sh"
	MasterYamlCreatePath = "./scripts/_create_runtime_master.sh"

	YamlBasePath = "./deploy/template/"

	HorizontalFl = "horizontal"
	VerticalFl   = "vertical"

	// algorithms

	LogisticRegressionKey = "logistic_regression"
	RuntimeLogs           = "runtime_logs"
)

var (
	// For user defined variables, define them in userdefined.properties first,
	// and then, add to here

	// meta env vars
	Env         = ""
	ServiceName = ""
	LogPath     = ""

	// Coord user-defined variables
	CoordIP       = ""
	CoordPort     = ""
	CoordBasePath = ""
	// later concatenated by falcon_platform.go
	CoordAddr = ""
	// number of consumers used in coord http server
	NbConsumers = ""
	// enable other service access master with clusterIP+clusterPort, from inside the cluster
	CoordK8sSvcName = ""

	// PartyServer user-define variables
	PartyServerIP       = ""
	PartyServerPort     = ""
	PartyID             = ""
	PartyServerBasePath = ""

	// JobDB and Database Configs
	// for dev use of sqlite3
	JobDatabase   = ""
	JobDbSqliteDb = ""
	// for prod use of mysql
	JobDbHost         = ""
	JobDbMysqlUser    = ""
	JobDbMysqlPwd     = ""
	JobDbMysqlDb      = ""
	JobDbMysqlOptions = ""
	// find the cluster port, call internally
	JobDbMysqlPort = ""
	// TODO: what is this?
	JobDbMysqlNodePort = ""

	// redis
	RedisHost = ""
	RedisPwd  = ""
	// find the cluster port, call internally
	RedisPort     = ""
	RedisNodePort = ""

	// those are init by falcon_platform
	WorkerType = ""
	WorkerAddr = ""
	MasterAddr = ""

	// this is the worker's k8s service name, only used in production
	WorkerK8sSvcName = ""

	MasterQItem = ""

	// paths used in training
	TaskDataPath    = ""
	TaskDataOutput  = ""
	TaskModelPath   = ""
	TaskRuntimeLogs = ""

	// paths for MPC and FL Engine
	MpcExePath   = ""
	FLEnginePath = ""
)

// GetEnv get key environment variable if exist otherwise return defalutValue
func GetEnv(key, defaultValue string) string {
	value := os.Getenv(key)
	if len(value) == 0 {
		logger.Log.Printf("Set env var to default {%s: %s}\n", key, defaultValue)
		return defaultValue
	}
	logger.Log.Printf("Read user defined env var {%s: %s}\n", key, value)
	return value
}
