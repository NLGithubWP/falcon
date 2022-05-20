package task

import (
	"falcon_platform/common"
	"falcon_platform/jobmanager/entity"
	"falcon_platform/logger"
	"fmt"
	"os"
	"os/exec"
)

type LimeWeightTask struct {
	DistributedRole uint
	WorkerID        common.WorkerIdType
	DslObj          *entity.DslObj4SingleWorker
}

//GetCommand
//	FL Engine requires:
//	("party-id", po::value<int>(&party_id), "current party id")
//	("party-num", po::value<int>(&party_num), "total party num")
//	("party-type", po::value<int>(&party_type), "type of this party, active or passive")
//	("fl-setting", po::value<int>(&fl_setting), "federated learning setting, horizontal or vertical")
//	("network-file", po::value<std::string>(&network_file), "file name of network configurations")
//	("log-file", po::value<std::string>(&log_file), "file name of log destination")
//	("data-input-file", po::value<std::string>(&data_input_file), "input file name of dataset")
//	("data-output-file", po::value<std::string>(&data_output_file), "output file name of dataset")
//	("existing-key", po::value<int>(&use_existing_key), "whether use existing phe keys")
//	("key-file", po::value<std::string>(&key_file), "file name of phe keys")
//	("algorithm-name", po::value<std::string>(&algorithm_name), "algorithm to be run")
//	// algorithm-params is not needed in inference stage
//	("algorithm-params", po::value<std::string>(&algorithm_params), "parameters for the algorithm")
//	("model-save-file", po::value<std::string>(&model_save_file), "model save file name")
//	// model-report-file is not needed in inference stage
//	("model-report-file", po::value<std::string>(&model_report_file), "model report file name")
//	("is-inference", po::value<int>(&is_inference), "whether it is an inference job")
//	("inference-endpoint", po::value<std::string>(&inference_endpoint), "endpoint to listen inference request");
//	("is-distributed", po::value<int>(&is_distributed), "is distributed");
//	("distributed-train-network-file", po::value<string>(&distributed_network_file), "ps network file");
//	("worker-id", po::value<int>(&worker_id), "worker id");
//	("distributed-role", po::value<int>(&distributed_role), "distributed role, worker:1, parameter server:0");

func (this *LimeWeightTask) GetCommand() *exec.Cmd {

	logger.Log.Println("[TrainWorker]: begin task RunLimeWeight")

	partyType := common.ConvertPartyType2Int(this.DslObj.PartyInfo.PartyType)
	flSetting := common.ConvertPartyType2Int(this.DslObj.JobFlType)

	// 3. generate many files store etc
	modelInputFile := common.TaskDataPath + "/" + this.DslObj.Tasks.LimeWeight.InputConfigs.DataInput.Data
	modelFile := common.TaskModelPath + "/" + this.DslObj.Tasks.LimeWeight.OutputConfigs.TrainedModel
	logFile := common.TaskRuntimeLogs + "-" + this.DslObj.Tasks.LimeWeight.AlgorithmName
	KeyFile := common.TaskDataPath + "/" + this.DslObj.Tasks.LimeWeight.InputConfigs.DataInput.Key
	modelReportFile := common.TaskModelPath + "/" + this.DslObj.Tasks.LimeWeight.OutputConfigs.EvaluationReport

	// 3. generate command line
	var usedLogFile string
	var distNetworkCfg string
	// in distributed training situation and this worker is parameter server
	if this.DslObj.DistributedTask.Enable == 1 && this.DistributedRole == common.DistributedParameterServer {
		logger.Log.Println("[PartyServer]: distributed training method with current distributed role: ", common.DistributedParameterServer)
		usedLogFile = logFile + "/parameter_server"
		distNetworkCfg = this.DslObj.DistributedExecutorPairNetworkCfg
	}

	// in distributed training situation and this worker is train worker
	if this.DslObj.DistributedTask.Enable == 1 && this.DistributedRole == common.DistributedWorker {
		logger.Log.Println("[PartyServer]: distributed training method with current distributed role: ", common.DistributedWorker)
		usedLogFile = logFile + "/distributed_worker_" + fmt.Sprintf("%d", this.WorkerID)
		distNetworkCfg = this.DslObj.DistributedExecutorPairNetworkCfg
	}

	// in centralized training
	if this.DslObj.DistributedTask.Enable == 0 {
		logger.Log.Println("[PartyServer]: training method: centralized")
		usedLogFile = logFile + "/centralized_worker"
		distNetworkCfg = common.EmptyParams
	}

	// create log folder for this task
	ee := os.MkdirAll(usedLogFile, os.ModePerm)
	if ee != nil {
		logger.Log.Fatalln("[PartyServer]: Creating distributed worker folder error", ee)
	}

	cmd := exec.Command(
		common.FLEnginePath,
		"--party-id", fmt.Sprintf("%d", this.DslObj.PartyInfo.ID),
		"--party-num", fmt.Sprintf("%d", this.DslObj.PartyNums),
		"--party-type", fmt.Sprintf("%d", partyType),
		"--fl-setting", fmt.Sprintf("%d", flSetting),
		"--network-file", this.DslObj.ExecutorPairNetworkCfg,
		"--log-file", usedLogFile,
		"--data-input-file", modelInputFile,
		"--data-output-file", common.TaskDataOutput,
		"--existing-key", fmt.Sprintf("%d", this.DslObj.ExistingKey),
		"--key-file", KeyFile,
		"--algorithm-name", this.DslObj.Tasks.LimeWeight.AlgorithmName,
		"--algorithm-params", this.DslObj.Tasks.LimeWeight.InputConfigs.SerializedAlgorithmConfig,
		"--model-save-file", modelFile,
		"--model-report-file", modelReportFile,
		"--is-inference", fmt.Sprintf("%d", 0),
		"--inference-endpoint", common.EmptyParams,
		"--is-distributed", fmt.Sprintf("%d", this.DslObj.DistributedTask.Enable),
		"--distributed-train-network-file", distNetworkCfg,
		"--worker-id", fmt.Sprintf("%d", int(this.WorkerID)%this.DslObj.WorkerPreGroup),
		"--distributed-role", fmt.Sprintf("%d", this.DistributedRole),
	)

	logger.Log.Printf("---------------------------------------------------------------------------------\n")
	logger.Log.Printf("[TrainWorker]: cmd is \"%s\"\n", cmd.String())
	logger.Log.Printf("---------------------------------------------------------------------------------\n")

	return cmd

}
