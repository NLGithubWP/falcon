package task

import (
	"falcon_platform/common"
	"falcon_platform/jobmanager/entity"
	"falcon_platform/logger"
	"fmt"
	"os"
	"os/exec"
)

type PreProcessTask struct {
	DistributedRole uint
	WorkerID        common.WorkerIdType
	DslObj          *entity.DslObj4SingleWorker
}

// GetCommand FL Engine requires:
//		("party-id", po::value<int>(&party_id), "current party id")
//		("party-num", po::value<int>(&party_num), "total party num")
//		("party-type", po::value<int>(&party_type), "type of this party, active or passive")
//		("fl-setting", po::value<int>(&fl_setting), "federated learning setting, horizontal or vertical")
//		("network-file", po::value<std::string>(&network_file), "file name of network configurations")
//		("log-file", po::value<std::string>(&log_file), "file name of log destination")
//		("data-input-file", po::value<std::string>(&data_input_file), "input file name of dataset")
//		("data-output-file", po::value<std::string>(&data_output_file), "output file name of dataset")
//		("existing-key", po::value<int>(&use_existing_key), "whether use existing phe keys")
//		("key-file", po::value<std::string>(&key_file), "file name of phe keys")
//		("algorithm-name", po::value<std::string>(&algorithm_name), "algorithm to be run")
//		// algorithm-params is not needed in inference stage
//		("algorithm-params", po::value<std::string>(&algorithm_params), "parameters for the algorithm")
//		("model-save-file", po::value<std::string>(&model_save_file), "model save file name")
//		// model-report-file is not needed in inference stage
//		("model-report-file", po::value<std::string>(&model_report_file), "model report file name")
//		("is-inference", po::value<int>(&is_inference), "whether it is an inference job")
//		("inference-endpoint", po::value<std::string>(&inference_endpoint), "endpoint to listen inference request");
//		("is-distributed", po::value<int>(&is_distributed), "is distributed");
//		("distributed-train-network-file", po::value<string>(&distributed_network_file), "ps network file");
//		("worker-id", po::value<int>(&worker_id), "worker id");
//		("distributed-role", po::value<int>(&distributed_role), "distributed role, worker:1, parameter server:0");
func (this *PreProcessTask) GetCommand() *exec.Cmd {

	logger.Log.Println("[TrainWorker]: begin task pre-processing")

	partyType := common.ConvertPartyType2Int(this.DslObj.PartyInfo.PartyType)
	flSetting := common.ConvertPartyType2Int(this.DslObj.JobFlType)

	// 3. generate many files store etc
	dataInputFile := common.TaskDataPath + "/" + this.DslObj.Tasks.PreProcessing.InputConfigs.DataInput.Data
	dataOutFile := common.TaskDataOutput + "/" + this.DslObj.Tasks.PreProcessing.OutputConfigs.DataOutput
	logFile := common.TaskRuntimeLogs + "-" + this.DslObj.Tasks.PreProcessing.AlgorithmName
	KeyFile := common.TaskDataPath + "/" + this.DslObj.Tasks.PreProcessing.InputConfigs.DataInput.Key

	_ = os.MkdirAll(logFile, os.ModePerm)

	// 3. generate command line
	// this is not defined yet, since there is no such task right now
	cmd := exec.Command(
		common.FLEnginePath,
		"--party-id", fmt.Sprintf("%d", this.DslObj.PartyInfo.ID),
		"--party-num", fmt.Sprintf("%d", this.DslObj.PartyNums),
		"--party-type", fmt.Sprintf("%d", partyType),
		"--fl-setting", fmt.Sprintf("%d", flSetting),
		"--existing-key", fmt.Sprintf("%d", this.DslObj.ExistingKey),
		"--key-file", KeyFile,
		"--network-file", this.DslObj.ExecutorPairNetworkCfg,

		"--algorithm-name", this.DslObj.Tasks.PreProcessing.AlgorithmName,
		"--algorithm-params", this.DslObj.Tasks.PreProcessing.InputConfigs.SerializedAlgorithmConfig,
		"--log-file", logFile,
		"--data-input-file", dataInputFile,
		"--data-output-file", dataOutFile,
		"--model-save-file", "",
		"--model-report-file", "",
	)

	logger.Log.Printf("---------------------------------------------------------------------------------\n")
	logger.Log.Printf("[TrainWorker]: cmd is \"%s\"\n", cmd.String())
	logger.Log.Printf("---------------------------------------------------------------------------------\n")

	// 4. execute cmd
	logger.Log.Println("[TrainWorker]: task pre processing start")

	return cmd
}
