package worker

import (
	"coordinator/common"
	"coordinator/distributed/entity"
	"coordinator/distributed/taskmanager"
	"coordinator/logger"
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"sync"
	"time"
)

func (wk *TrainWorker) TrainTask(doTaskArgs *entity.DoTaskArgs, rep *entity.DoTaskReply) {

	wg := sync.WaitGroup{}

	wg.Add(1)

	// no need to wait for mpc, once train task done, shutdown the mpc
	go wk.mpcTaskCallee(doTaskArgs, "algName")
	go wk.mlTaskCallee(doTaskArgs, rep, &wg)

	// wait until all the task done
	wg.Wait()

	// kill all the monitors, which will cause to kill all running sub processes
	wk.Pm.Cancel()

	wk.Pm.Lock()
	rep.Killed = wk.Pm.IsKilled
	if wk.Pm.IsKilled == true {
		wk.Pm.Unlock()
		wk.TaskFinish <- true
	} else {
		wk.Pm.Unlock()
	}
}

func (wk *TrainWorker) mlTaskCallee(doTaskArgs *entity.DoTaskArgs, rep *entity.DoTaskReply, wg *sync.WaitGroup) {
	/**
	 * @Author
	 * @Description
	 * @Date 2:52 下午 12/12/20
	 * @Param
		requires:

			("party-id", po::value<int>(&party_id), "current party id")
			("party-num", po::value<int>(&party_num), "total party num")
			("party-type", po::value<int>(&party_type), "type of this party, active or passive")
			("fl-setting", po::value<int>(&fl_setting), "federated learning setting, horizontal or vertical")

			("network-file", po::value<std::string>(&network_file), "file name of network configurations")
			("log-file", po::value<std::string>(&log_file), "file name of log destination")
			("data-input-file", po::value<std::string>(&data_file), "file name of dataset")
			("data-output-file", po::value<std::string>(&data_file), "file name of dataset")
			("model-save-file", po::value<std::string>(&data_file), "file name of dataset")
			("key-file", po::value<std::string>(&key_file), "file name of phe keys")

			("existing-key", po::value<int>(&use_existing_key), "whether use existing phe keys")
			("algorithm-name", po::value<std::string>(&algorithm_name), "algorithm to be run")
			("algorithm-params", po::value<std::string>(&algorithm_params), "parameters for the algorithm");

	 * @return
	 **/
	defer wg.Done()

	logger.Do.Printf("Worker: %s task started \n", wk.Addr)

	partyId := doTaskArgs.AssignID
	partyNum := doTaskArgs.PartyNums
	partyType := 1
	partyTypeStr := doTaskArgs.PartyInfo.PartyType
	// TODO: check with wyc if partyType 0 = active?
	if partyTypeStr == "active" {
		partyType = 0
	} else if partyTypeStr == "passive" {
		partyType = 1
	}
	flSetting := 1
	flSettingStr := doTaskArgs.JobFlType
	if flSettingStr == "vertical" {
		flSetting = 1
	} else if flSettingStr == "horizontal" {
		flSetting = 0
	}
	existingKey := doTaskArgs.ExistingKey

	dataInputFile := common.TaskDataPath + "/" + doTaskArgs.TaskInfo.PreProcessing.InputConfigs.DataInput.Data
	dataOutFile := common.TaskDataOutput + "/" + doTaskArgs.TaskInfo.PreProcessing.OutputConfigs.DataOutput
	modelInputFile := common.TaskDataOutput + "/" + doTaskArgs.TaskInfo.ModelTraining.InputConfigs.DataInput.Data
	modelFile := common.TaskModelPath + "/" + doTaskArgs.TaskInfo.ModelTraining.OutputConfigs.TrainedModel

	modelReportFile := common.TaskModelPath + "/" + doTaskArgs.TaskInfo.ModelTraining.OutputConfigs.EvaluationReport

	var algParams string
	var KeyFile string

	run := doMlTask(
		wk.Pm,
		fmt.Sprintf("%d", partyId),
		fmt.Sprintf("%d", partyNum),
		fmt.Sprintf("%d", partyType),
		fmt.Sprintf("%d", flSetting),
		fmt.Sprintf("%d", existingKey),
		doTaskArgs.NetWorkFile,
	)

	var exitStr string
	var res map[string]string
	var logFile string
	fmt.Println(res)

	if doTaskArgs.TaskInfo.PreProcessing.AlgorithmName != "" {
		logger.Do.Println("Worker:task 1 pre processing start")
		logFile = common.TaskRuntimeLogs + "/" + doTaskArgs.TaskInfo.PreProcessing.AlgorithmName
		_ = os.Mkdir(logFile, os.ModePerm)
		KeyFile = doTaskArgs.TaskInfo.PreProcessing.InputConfigs.DataInput.Key

		algParams = doTaskArgs.TaskInfo.PreProcessing.InputConfigs.SerializedAlgorithmConfig
		exitStr, res = run(
			doTaskArgs.TaskInfo.PreProcessing.AlgorithmName,
			algParams,
			KeyFile,
			logFile,
			dataInputFile,
			dataOutFile,
			"", "",
		)
		if exit := wk.execResHandler(exitStr, res, rep); exit == true {
			return
		}
		logger.Do.Println("Worker:task 1 pre processing done", rep)
	}

	if doTaskArgs.TaskInfo.ModelTraining.AlgorithmName != "" {
		// execute task 2: train
		logger.Do.Println("Worker:task model training start")
		logFile = common.TaskRuntimeLogs + "/" + doTaskArgs.TaskInfo.ModelTraining.AlgorithmName
		_ = os.Mkdir(logFile, os.ModePerm)
		KeyFile = doTaskArgs.TaskInfo.ModelTraining.InputConfigs.DataInput.Key

		algParams = doTaskArgs.TaskInfo.ModelTraining.InputConfigs.SerializedAlgorithmConfig
		logger.Do.Println("Worker: SerializedAlgorithmConfig is", algParams)
		exitStr, res = run(
			doTaskArgs.TaskInfo.ModelTraining.AlgorithmName,
			algParams,
			KeyFile,
			logFile,
			modelInputFile,
			"thisIsDataOutput",
			modelFile,
			modelReportFile,
		)

		if exit := wk.execResHandler(exitStr, res, rep); exit == true {
			return
		}
		logger.Do.Println("Worker:task model training", rep)
	}

	// 2 thread will ready from isStop channel, only one is running at the any time
}

func (wk *TrainWorker) mpcTaskCallee(doTaskArgs *entity.DoTaskArgs, algName string) {
	/**
	 * @Author
	 * @Description
	 * @Date 2:52 下午 12/12/20
	 * @Param

		./semi-party.x -F -N 3 -I -p 0 -h 10.0.0.33 -pn 6000 algorithm_name
			-N party_num
			-p party_id
			-h ip
			-pn port
			algorithm_name

		-h 是party_0的ip 端口目前只有一个 各个端口都相同就可以
		-h 每个mpc进程的启动输入都是party_0的ip
	 * @return
	 **/
	partyId := doTaskArgs.AssignID
	partyNum := doTaskArgs.PartyNums

	var envs []string

	cmd := exec.Command(
		common.MpcExe,
		" --F ",
		" --N  "+fmt.Sprintf("%d", partyNum),
		" --I ",
		" --p "+fmt.Sprintf("%d", partyId),
		" --h "+doTaskArgs.MpcIp,
		" --pn "+fmt.Sprintf("%d", doTaskArgs.MpcPort),
		" "+algName,
	)

	logger.Do.Println(envs, cmd.String())
	logger.Do.Println("mpcTask Done!")

	//wk.Pm.CreateResources(cmd, envs)
	return
}

func (wk *TrainWorker) execResHandler(
	exitStr string,
	RuntimeErrorMsg map[string]string,
	rep *entity.DoTaskReply) bool {
	/**
	 * @Author
	 * @Description
	 * @Date 3:55 下午 14/12/20
	 * @Param
			true: has error, exit
			false: no error, keep running
	 * @return
	 **/
	js, err := json.Marshal(RuntimeErrorMsg)
	if err != nil {
		logger.Do.Println("Worker: Serialize job status error", err)
		return true
	}
	rep.TaskMsg.RuntimeMsg = string(js)

	if exitStr != common.SubProcessNormal {
		rep.RuntimeError = true
		// return is used to control the rpc call status, always return nil, but
		// keep error at rep.ErrorMsg
		return true
	}

	rep.RuntimeError = false
	return false
}

func TestTaskProcess(doTaskArgs *entity.DoTaskArgs) {

	partyId := doTaskArgs.AssignID
	partyNum := doTaskArgs.PartyNums
	partyType := 1
	partyTypeStr := doTaskArgs.PartyInfo.PartyType
	// TODO: check with wyc if partyType 0 = active?
	if partyTypeStr == "active" {
		partyType = 0
	} else if partyTypeStr == "passive" {
		partyType = 1
	}
	flSetting := 1
	flSettingStr := doTaskArgs.JobFlType
	if flSettingStr == "vertical" {
		flSetting = 1
	} else if flSettingStr == "horizontal" {
		flSetting = 0
	}
	existingKey := doTaskArgs.ExistingKey
	//dataInputFile := common.TaskDataPath +"/" + doTaskArgs.TaskInfo.PreProcessing.InputConfigs.DataInput.Data
	modelFile := common.TaskModelPath + "/" + doTaskArgs.TaskInfo.ModelTraining.OutputConfigs.TrainedModel
	algParams := doTaskArgs.TaskInfo.ModelTraining.InputConfigs.SerializedAlgorithmConfig
	logger.Do.Println("Worker: SerializedAlgorithmConfig is", algParams)

	modelReportFile := common.TaskModelPath + "/" + doTaskArgs.TaskInfo.ModelTraining.OutputConfigs.EvaluationReport
	logFile := common.TaskRuntimeLogs + "/" + doTaskArgs.TaskInfo.PreProcessing.AlgorithmName
	KeyFile := doTaskArgs.TaskInfo.PreProcessing.InputConfigs.DataInput.Key
	modelInputFile := common.TaskDataOutput + "/" + doTaskArgs.TaskInfo.ModelTraining.InputConfigs.DataInput.Data

	logger.Do.Printf("--------------------------------------------------\n")
	logger.Do.Printf("\n")
	logger.Do.Println("executed path is: ", strings.Join([]string{
		common.FalconTrainExe,
		" --party-id " + fmt.Sprintf("%d", partyId),
		" --party-num " + fmt.Sprintf("%d", partyNum),
		" --party-type " + fmt.Sprintf("%d", partyType),
		" --fl-setting " + fmt.Sprintf("%d", flSetting),
		" --existing-key " + fmt.Sprintf("%d", existingKey),
		" --key-file " + KeyFile,
		" --network-file " + doTaskArgs.NetWorkFile,

		" --algorithm-name " + doTaskArgs.TaskInfo.ModelTraining.AlgorithmName,
		" --algorithm-params " + algParams,
		" --log-file " + logFile,
		" --data-input-file " + modelInputFile,
		" --data-output-file ",
		" --model-save-file " + modelFile,
		" --model-report-file " + modelReportFile,
	}, " "))
	logger.Do.Printf("\n")
	logger.Do.Printf("--------------------------------------------------\n")
	time.Sleep(time.Minute)
}

func doMlTask(
	pm *taskmanager.SubProcessManager,

	partyId string,
	partyNum string,
	partyType string,
	flSetting string,
	existingKey string,

	netFile string,

) func(string, string, string, string, string, string, string, string) (string, map[string]string) {
	// WTF was that??! WTF were all those strings???!
	/**
	 * @Author
	 * @Description  record if the task is fail or not
	 * @Date 7:32 下午 12/12/20
	 * @Param key, algorithm name, value: error message
	 * @return
	 **/

	// Closure
	res := make(map[string]string)

	return func(
		algName string,
		algParams string,

		KeyFile string,
		logFile string,
		dataInputFile string,
		dataOutputFile string,
		modelSaveFile string,
		modelReport string,
	) (string, map[string]string) {

		var envs []string

		cmd := exec.Command(
			common.FalconTrainExe,
			"--party-id", partyId,
			"--party-num", partyNum,
			"--party-type", partyType,
			"--fl-setting", flSetting,
			"--existing-key", existingKey,
			"--key-file", KeyFile,
			"--network-file", netFile,

			"--algorithm-name", algName,
			"--algorithm-params", algParams,
			"--log-file", logFile,
			"--data-input-file", dataInputFile,
			"--data-output-file", dataOutputFile,
			"--model-save-file", modelSaveFile,
			"--model-report-file", modelReport,
		)

		logger.Do.Printf("-----------------------------------------------------------------\n")
		logger.Do.Printf("\n")
		logger.Do.Println("envs", envs)
		logger.Do.Println(cmd.String())
		logger.Do.Printf("\n")
		logger.Do.Printf("-----------------------------------------------------------------\n")

		exitStr, runTimeErrorLog := pm.CreateResources(cmd, envs)
		res[algName] = runTimeErrorLog
		return exitStr, res

	}
}
