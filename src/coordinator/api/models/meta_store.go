package models

import (
	"coordinator/config"
	"coordinator/logger"
	"fmt"
	"github.com/jinzhu/gorm"

	_ "gorm.io/driver/mysql"
	_ "gorm.io/driver/sqlite"

)

type MetaStore struct {
	engine   string
	host     string
	user     string
	password string
	database string
	url      string
	Db       *gorm.DB
	Tx       *gorm.DB
}

func InitMetaStore() *MetaStore {
	ms := new(MetaStore)
	ms.engine = config.MsEngine
	ms.host = config.MsHost
	ms.user = ""
	ms.password = ""
	ms.database = ""

	if ms.engine == "mysql" {
		ms.user = config.MsMysqlUser
		ms.password = config.MsMysqlPwd
		ms.database = config.MsMysqlDb

		mysql_url := fmt.Sprintf(
			"%s:%s@tcp(%s:3306)/%s%s",
			ms.user,
			ms.password,
			ms.host,
			ms.database,
			config.MsMysqlOptions,
		)
		ms.url = mysql_url
	} else if ms.engine == "sqlite3" {
		ms.url = config.MsSqliteDb
	}
	return ms
}

// connect to db, and begin the transaction
func (ms *MetaStore) Connect() {
	db, err := gorm.Open(ms.engine, ms.url)
	if err != nil {
		logger.Do.Println(err)
		return
	}
	ms.Db = db
}

// disconnect , should call after ms.Commit
func (ms *MetaStore) DisConnect() {

	e := ms.Db.Close()
	if e != nil {
		logger.Do.Println("closeDb error")
	}
}

func (ms *MetaStore) DefineTables() {

	if ms.Db.HasTable(&JobRecord{}) {
		ms.Db.AutoMigrate(&JobRecord{})
	} else {
		ms.Db.CreateTable(&JobRecord{})
	}

	if ms.Db.HasTable(&TaskRecord{}) {
		ms.Db.AutoMigrate(&TaskRecord{})
	} else {
		ms.Db.CreateTable(&TaskRecord{})
	}

	if ms.Db.HasTable(&ExecutionRecord{}) {
		ms.Db.AutoMigrate(&ExecutionRecord{})
	} else {
		ms.Db.CreateTable(&ExecutionRecord{})
	}

	if ms.Db.HasTable(&ModelRecord{}) {
		ms.Db.AutoMigrate(&ModelRecord{})
	} else {
		ms.Db.CreateTable(&ModelRecord{})
	}

	if ms.Db.HasTable(&ServiceRecord{}) {
		ms.Db.AutoMigrate(&ServiceRecord{})
	} else {
		ms.Db.CreateTable(&ServiceRecord{})
	}

	if ms.Db.HasTable(&ModelRecord{}) {
		ms.Db.AutoMigrate(&ModelRecord{})
	} else {
		ms.Db.CreateTable(&ModelRecord{})
	}

	if ms.Db.HasTable(&User{}) {
		ms.Db.AutoMigrate(&User{})
	} else {
		ms.Db.CreateTable(&User{})
	}

	if ms.Db.HasTable(&Listeners{}) {
		ms.Db.AutoMigrate(&Listeners{})
	} else {
		ms.Db.CreateTable(&Listeners{})
	}

	if ms.Db.HasTable(&TestTable{}) {
		ms.Db.AutoMigrate(&TestTable{})
	} else {
		ms.Db.CreateTable(&TestTable{})
	}
}

func (ms *MetaStore) Commit(el interface{}) {

	switch el.(type) {
	case []error:
		res, _ := el.([]error)
		for _, ev := range res {
			if ev != nil {
				logger.Do.Println("Sql error", ev)
				ms.Tx.Rollback()
				panic(ev)
			}
		}
		ms.Tx.Commit()
	case error:
		res, _ := el.(error)
		if res != nil {
			logger.Do.Println("Sql error", res)
			ms.Tx.Rollback()
			panic(res)
		}
		ms.Tx.Commit()
	}
}

////////////////////////////////////
/////////// Test falcon_sql ////////////
////////////////////////////////////

func (ms *MetaStore) InsertTest(name string) (error, *TestTable) {
	u := &TestTable{Name: name}

	err := ms.Db.Create(u).Error
	return err, u
}
