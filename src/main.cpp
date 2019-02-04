#ifdef WIN32
#	include <Windows.h>
#endif
#include <curl/curl.h>
#include <iconv.h>
#include <errno.h>

#include "utils/config/config.h"
#include "utils/dbpool/dbpool.h"
#include "utils/log/log.h"

enum ESettingsType {
	m_eInternet = 1,
	m_eMMS = 2,
	m_eInternetMMS = 3
};

enum EProfile {
	m_eUserPin = 1,
	m_eNetwPin = 2,
	m_eSimpleText = 3,
	m_eSimpleWnd = 4,
	m_eHonor = 5
};

const char *g_pszNotiInet     = "Автоматические настройки мобильного интернета будут доставлены следующим конфигурационным SMS";
const char *g_pszNotiMMS      = "Автоматические настройки MMS будут доставлены следующим конфигурационным SMS";
const char *g_pszNotiInetMMS  = "Автоматические настройки мобильного интернета и MMS будут доставлены следующим конфигурационным SMS";
const char *g_pszNotiOpt      = ". При установке настроек используйте PIN-код 1234";

const char *g_pszNotiPostInet     = "Конфигурационное SMS доставлено. Проведите его установку. При невозможности установки конфигурационного SMS, в меню \"Настройки\" выберите пункт \"Мобильные сети\", \"Точки доступа\" и разделе \"Интернет\" создайте новую точку доступа, введите параметры Имя: Internet, APN: internet.letai.ru. Перезагрузите Ваше устройство.";
const char *g_pszNotiPostMMS      = "Конфигурационное SMS доставлено. Проведите его установку. При невозможности установки конфигурационного SMS, в меню \"Настройки\" выберите пункт \"Мобильные сети\", \"Точки доступа\" и в разделе \"MMS\" создайте новую точку доступа, введите параметры Имя: MMS; APN(Точка доступа): mms; MMSC: https://mmsc:8002; MMS-прокси: mmsc; Порт MMS: 8080. Перезагрузите Ваше устройство.";
const char *g_pszNotiPostInetMMS  = "Конфигурационное SMS доставлено. Проведите его установку. При невозможности установки конфигурационного SMS, для настройки мобильного интернета в меню \"Настройки\" выберите пункт \"Мобильные сети\", \"Точки доступа\" и разделе \"Интернет\" создайте новую точку доступа, введите параметры Имя: Internet, APN: internet.letai.ru. Для настройки MMS в разделе \"MMS\" создайте новую точку доступа, введите параметры Имя: MMS; APN(Точка доступа): mms; MMSC: https://mmsc:8002; MMS-прокси: mmsc; Порт MMS: 8080. Перезагрузите Ваше устройство.";

const char *g_pszAPNInet  = "<?xml version=\"1.0\"?><!DOCTYPE wap-provisioningdoc PUBLIC \"-//WAPFORUM//DTD PROV 1.0//EN\" \"http://www.wapforum.org/DTD/prov.dtd\"><wap-provisioningdoc version=\"1.0\"><characteristic type=\"BOOTSTRAP\"><parm name=\"NAME\" value=\"Letai Internet\"/></characteristic><characteristic type=\"NAPDEF\"><parm name=\"NAPID\" value=\"NAP1\"/><parm name=\"NAME\" value=\"Letai Internet\"/><parm name=\"BEARER\" value=\"GSM-GPRS\"/><parm name=\"NAP-ADDRESS\" value=\"internet.letai.ru\"/><parm name=\"NAP-ADDRTYPE\" value=\"APN\"/></characteristic><characteristic type=\"APPLICATION\"><parm name=\"APPID\" value=\"w2\"/><parm name=\"NAME\" value=\"Letai Internet\"/><parm name=\"TO-NAPID\" value=\"NAP1\"/><characteristic type=\"RESOURCE\"><parm name=\"URI\" value=\"http://letai.ru\"/><parm name=\"NAME\" value=\"Letai Internet\"/><parm name=\"STARTPAGE\"/></characteristic></characteristic></wap-provisioningdoc>";
const char *g_pszAPNInetHonor = "<?xml version=\"1.0\"?><!DOCTYPE wap-provisioningdoc PUBLIC \"-//WAPFORUM//DTD PROV 1.0//EN\" \"http://www.wapforum.org/DTD/prov.dtd\"><wap-provisioningdoc version=\"1.0\"><characteristic type=\"NAPDEF\"><parm name=\"NAPID\" value=\"NAP1\"/><parm name=\"NAME\" value=\"Letai Internet\"/><parm name=\"BEARER\" value=\"GSM-GPRS\"/><parm name=\"NAP-ADDRESS\" value=\"internet.letai.ru\"/><parm name=\"NAP-ADDRTYPE\" value=\"APN\"/></characteristic><characteristic type=\"APPLICATION\"><parm name=\"APPID\" value=\"w2\"/><parm name=\"NAME\" value=\"Letai Internet\"/><parm name=\"TO-NAPID\" value=\"NAP1\"/><characteristic type=\"RESOURCE\"><parm name=\"URI\" value=\"http://letai.ru\"/><parm name=\"NAME\" value=\"Letai Internet\"/><parm name=\"STARTPAGE\"/></characteristic></characteristic></wap-provisioningdoc>";
const char *g_pszAPNMMS   = "<?xml version=\"1.0\"?><!DOCTYPE wap-provisioningdoc PUBLIC \"-//WAPFORUM//DTD PROV 1.0//EN\" \"http://www.wapforum.org/DTD/prov.dtd\"><wap-provisioningdoc version=\"1.0\"><characteristic type=\"BOOTSTRAP\"><parm name=\"NAME\" value=\"Letai MMS\"/></characteristic><characteristic type=\"PXLOGICAL\"><parm name=\"PROXY-ID\" value=\"Letai MMS\"/><parm name=\"NAME\" value=\"Letai MMS\"/><characteristic type=\"PXPHYSICAL\"><parm name=\"PHYSICAL-PROXY-ID\" value=\"PROXY 1\"/><parm name=\"PXADDR\" value=\"mmsc\"/><parm name=\"PXADDRTYPE\" value=\"ALPHA\"/><parm name=\"TO-NAPID\" value=\"NAP5\"/><characteristic type=\"PORT\"><parm name=\"PORTNBR\" value=\"8080\"></parm></characteristic></characteristic></characteristic><characteristic type=\"NAPDEF\"><parm name=\"NAPID\" value=\"NAP5\"/><parm name=\"NAME\" value=\"Letai MMS\"/><parm name=\"BEARER\" value=\"GSM-GPRS\"/><parm name=\"NAP-ADDRESS\" value=\"mms\"/><parm name=\"NAP-ADDRTYPE\" value=\"APN\"/></characteristic><characteristic type=\"APPLICATION\"><parm name=\"APPID\" value=\"w4\"/><parm name=\"NAME\" value=\"Letai MMS\"/><parm name=\"ADDR\" value=\"http://mmsc\"/><parm name=\"TO-PROXY\" value=\"Letai MMS\"/></characteristic></wap-provisioningdoc>";

const char *g_pszTextInet = "Настройки мобильного интернета. В меню \"Настройки\" выберите пункт \"Сотовая связь\", \"Сотовая сеть передачи данных\" и в разделе \"Сотовые данные\" заполните параметры: Имя: letai; APN: internet.letai.ru; Тип APN: default; Протокол роуминга APN: IPv4";
const char *g_pszTextMMS  = "Настройки MMS. В меню \"Настройки\" выберите пункт \"Сотовая связь\", \"Сотовая сеть передачи данных\" и в разделе \"MMS\" заполните параметры: Имя: letai mms; APN: mms; MMSC: http://mmsc; Прокси-сервер: mmsc; Порт MMS: 8080; Тип APN: mms. Перезагрузите Ваше устройство.";

const char *g_pszTextInetWnd = "Настройки мобильного интернета. В меню \"Настройки\" выберите пункт \"Сотовая сеть\" и в разделе \"Точки интернет-доступа\" заполните параметр APN(Точка доступа): internet.letai.ru";
const char *g_pszTextMMSWnd  = "Настройки MMS. В меню \"Настройки\" выберите пункт \"Сотовая сеть\", в разделе \"Настройки SIM-карты\" добавьте новую точку доступа MMS и заполните параметры APN(Точка доступа): mms; MMS-центр: https://mmsc:8002; Порт MMS: 8080. Перезагрузите Ваше устройство.";

int is_ascii_string(
	const char *p_pszString,
	size_t p_stStrLen);
int conv_string_to_ucs2(
	CLog &p_coLog,
	std::string &p_strIn,
	std::string &p_strOut);
int put_sms(
	CLog &p_coLog,
	std::string p_strHost,
	std::string p_strURL,
	std::string p_strUserName,
	std::string p_strUserPswd,
	std::string p_strFrom,
	std::string p_strTo,
	std::string p_strText);
int put_ota(
	CLog &p_coLog,
	std::string &p_strHost,
	std::string &p_strURL,
	std::string &p_strUserName,
	std::string &p_strUserPswd,
	std::string &p_strFrom,
	std::string &p_strTo,
	std::string &p_strText,
	std::string &p_strSec,
	std::string &p_strPin);
void append_urlparam(
	CURL *pCurl,
	std::string &p_strParamSet,
	const char *p_pszParamName,
	std::string &p_strParam,
  bool p_bValueURLEncode,
  bool &p_bFirstParam);
void IMSI2SemiOctet(
	std::string &p_strIMSI,
	std::string &p_strSemiOctet);

int main(int argc, char *argv[])
{
	int iRetVal = 0;
	int iFnRes;
	CLog coLog;
	CConfig coConf;
	std::string strConfParamValue;
	std::string strSMSBoxUserName;
	std::string strSMSBoxUserPswd;
	std::string strSMSBoxHost;
	std::string strSMSBoxOTAURL;
	std::string strSMSBoxSMSURL;
	const char *pszParamName = NULL;
	ESettingsType eSettingsType;
	EProfile eProfile;

	/* проверяем наличие параметра в командной строке */
	if (argc < 2) {
		iRetVal = -1;
		UTL_LOG_E(coLog, "return code: '%d';", iRetVal);
		return iRetVal;
	}
	/* загружаем конфигурацию */
	iFnRes = coConf.LoadConf(argv[1]);
	if (iFnRes) {
		iRetVal = -2;
		UTL_LOG_E(coLog, "return code: '%d';", iRetVal);
		return iRetVal;
	}
	/* инициализируем логгер */
	iFnRes = coConf.GetParamValue("log_file_mask", strConfParamValue);
	if (iFnRes) {
		iRetVal = -3;
		UTL_LOG_E(coLog, "return code: '%d';", iRetVal);
		return iRetVal;
	}
	iFnRes = coLog.Init(strConfParamValue.c_str());
	if (iFnRes) {
		iRetVal = -4;
		UTL_LOG_E(coLog, "return code: '%d';", iRetVal);
		return iRetVal;
	}
	/* инициализация сurl */
	curl_global_init(0);
	/* инициализируем пул подключений к БД */
	std::string strDBUserName, strDBUserPswd, strDBDescr;
	/* запрашиваем необходимые параметры */
	pszParamName = "db_user";
	iFnRes = coConf.GetParamValue(pszParamName, strDBUserName);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
	pszParamName = "db_pswd";
	iFnRes = coConf.GetParamValue(pszParamName, strDBUserPswd);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
	pszParamName = "db_descr";
	iFnRes = coConf.GetParamValue(pszParamName, strDBDescr);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
  /* подключение к БД */
	iFnRes = db_pool_init(&coLog, strDBUserName, strDBUserPswd, strDBDescr, 1);
	if (iFnRes) {
		iRetVal = -5;
		UTL_LOG_E(coLog, "return code: '%d';", iRetVal);
		return iRetVal;
	}

	otl_stream coQueue;
	otl_stream coResult;
	otl_stream coDelete;
	otl_connect *pcoDBConn = NULL;
	pcoDBConn = db_pool_get();

	/* запрашиваем необходимые параметры */
	pszParamName = "smsbox_username";
	iFnRes = coConf.GetParamValue(pszParamName, strSMSBoxUserName);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
	pszParamName = "smsbox_userpswd";
	iFnRes = coConf.GetParamValue(pszParamName, strSMSBoxUserPswd);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
	pszParamName = "smsbox_host";
	iFnRes = coConf.GetParamValue(pszParamName, strSMSBoxHost);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
	pszParamName = "smsbox_url_ota";
	iFnRes = coConf.GetParamValue(pszParamName, strSMSBoxOTAURL);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}
	pszParamName = "smsbox_url_sms";
	iFnRes = coConf.GetParamValue(pszParamName, strSMSBoxSMSURL);
	if (iFnRes) {
		UTL_LOG_W(coLog, "parameter '%s' is not defined in configuration", pszParamName);
	}

	if (pcoDBConn) {
		try {
			otl_value<std::string> coRowId;
			otl_value<std::string> coMSISDN;
			otl_value<std::string> coIMSI;
			otl_value<std::string> coProfile;
			otl_value<std::string> coSettingsType;
      std::string strHeader = "116111";
			std::string strText;
			std::string strSec;
			std::string strPin;
			coQueue.open(50, "select rowid, msisdn, imsi, profile, settings_type from ps.omaQueue", *pcoDBConn);
			coResult.open(
				1,
				"insert into ps.omaResult (msisdn,header,message,sec,pin,status,sent) values "
				"(:msisdn/*char[12]*/,:header/*char[12]*/,:message/*char[2000]*/,:sec/*char[32]*/,:pin/*char[32]*/,:status/*int*/,sysdate)",
				*pcoDBConn);
			coDelete.open(1, "delete from ps.omaQueue where rowid = :row_id/*char[256]*/", *pcoDBConn);
			while (!coQueue.eof()) {
        strText.clear ();
				coQueue
					>> coRowId
					>> coMSISDN
					>> coIMSI
					>> coProfile
					>> coSettingsType;
        UTL_LOG_N (coLog, "row selected: '%s'; '%s'; '%s'; '%s'", coMSISDN.v.c_str(), coIMSI.v.c_str(), coProfile.v.c_str(), coSettingsType.v.c_str());
        if (coMSISDN.is_null ()) {
					UTL_LOG_E(coLog, "MSISDN not defined");
					goto delete_and_continue;
				} else {
					if (coMSISDN.v[0] != '+')
						coMSISDN.v = '+' + coMSISDN.v;
				}
				if (coProfile.is_null()) {
					UTL_LOG_E(coLog, "profile not defined");
					goto delete_and_continue;;
				}
				if (coSettingsType.is_null()) {
					UTL_LOG_E(coLog, "settings type not defined");
					goto delete_and_continue;;
				}
				if (0 == coProfile.v.compare("OMA CP NETWPIN")) {
					eProfile = m_eNetwPin;
				} else if (0 == coProfile.v.compare("OMA CP USERPIN")) {
					eProfile = m_eUserPin;
				} else if (0 == coProfile.v.compare("SIMPLE TEXT")) {
					eProfile = m_eSimpleText;
				} else if (0 == coProfile.v.compare("SIMPLE TEXT WND")) {
					eProfile = m_eSimpleWnd;
				} else if ( 0 == coProfile.v.compare( "HONOR" ) ) {
          eProfile = m_eHonor;
        } else {
					UTL_LOG_E(coLog, "unsupported profile: '%s'", coProfile.v.c_str());
					goto delete_and_continue;;
				}

				if (0 == coSettingsType.v.compare("INTERNET")) {
					eSettingsType = m_eInternet;
				} else if (0 == coSettingsType.v.compare("MMS")) {
					eSettingsType = m_eMMS;
				} else if (0 == coSettingsType.v.compare("INTERNET+MMS")) {
					eSettingsType = m_eInternetMMS;
				} else {
					UTL_LOG_E(coLog, "unsupported settings type: '%s'", coSettingsType.v.c_str());
					goto delete_and_continue;;
				}

        /* отправляем первичное уведомление */
				switch (eSettingsType) {
				case m_eInternet:
					strText = g_pszNotiInet;
					break;
				case m_eMMS:
					strText = g_pszNotiMMS;
					break;
        case m_eInternetMMS:
					strText = g_pszNotiInetMMS;
					break;
				}

				switch (eProfile) {
				  case m_eUserPin:
          case m_eHonor:
					  strSec = "userpin";
					  strPin = "1234";
					  strText += g_pszNotiOpt;
					  break;
				  case m_eNetwPin:
					  strSec = "netwpin";
					  IMSI2SemiOctet(coIMSI.v, strPin);
					  break;
				  case m_eSimpleText:
          case m_eSimpleWnd:
            /* для текстовых настроек уведомление не посылается */
            strText.clear ();
					  break;
				}

        if (0 != strText.length()) {
				  iFnRes = put_sms(coLog, strSMSBoxHost, strSMSBoxSMSURL, strSMSBoxUserName, strSMSBoxUserPswd, strHeader, coMSISDN.v, strText);
				  UTL_LOG_N(coLog, "sms is sent with status '%d': '%s'; '%s'; '%s';", iFnRes, strHeader.c_str(), coMSISDN.v.c_str(), strText.c_str());
          try {
            coResult << coMSISDN << strHeader << strText << strSec << strPin << iFnRes;
          } catch (otl_exception &coExept) {
            UTL_LOG_E (coLog, "code: '%d'; message: '%s'; query: '%s';", coExept.code, coExept.msg, coExept.stm_text);
            coResult.clean();
          }
        }

				/* отправляем настройки INTERNET */
				switch (eSettingsType) {
				case m_eInternet:
				case m_eInternetMMS:
					if (eProfile == m_eSimpleText) {
						strText = g_pszTextInet;
          } else if (eProfile == m_eSimpleWnd) {
            strText = g_pszTextInetWnd;
          } else if (eProfile == m_eHonor) {
            strText = g_pszAPNInetHonor;
          } else {
						strText = g_pszAPNInet;
          }
					break;
				default:
          strText.clear ();
				}
				if (0 != strText.length()) {
          if (eProfile == m_eSimpleText || eProfile == m_eSimpleWnd) {
						iFnRes = put_sms(coLog, strSMSBoxHost, strSMSBoxSMSURL, strSMSBoxUserName, strSMSBoxUserPswd, strHeader, coMSISDN.v, strText);
          } else {
						iFnRes = put_ota(coLog, strSMSBoxHost, strSMSBoxOTAURL, strSMSBoxUserName, strSMSBoxUserPswd, strHeader, coMSISDN.v, strText, strSec, strPin);
          }
					UTL_LOG_N(coLog, "sms is sent with status '%d': '%s'; '%s'; '%s';", iFnRes, strHeader.c_str(), coMSISDN.v.c_str(), strText.c_str());
          try {
            coResult << coMSISDN << strHeader << strText << strSec << strPin << iFnRes;
          } catch ( otl_exception &coExept ) {
            UTL_LOG_E( coLog, "code: '%d'; message: '%s'; query: '%s';", coExept.code, coExept.msg, coExept.stm_text );
            coResult.clean();
          }
        }

				/* отправялем настройки MMS */
				switch (eSettingsType) {
				case m_eMMS:
				case m_eInternetMMS:
					if (eProfile == m_eSimpleText) {
						strText = g_pszTextMMS;
          } else if (eProfile == m_eSimpleWnd) {
						strText = g_pszTextMMSWnd;
          } else {
						strText = g_pszAPNMMS;
          }
					break;
				default:
          strText.clear ();
        }
				if (0 != strText.length()) {
          if (eProfile == m_eSimpleText || eProfile == m_eSimpleWnd)
						iFnRes = put_sms(coLog, strSMSBoxHost, strSMSBoxSMSURL, strSMSBoxUserName, strSMSBoxUserPswd, strHeader, coMSISDN.v, strText);
					else
						iFnRes = put_ota(coLog, strSMSBoxHost, strSMSBoxOTAURL, strSMSBoxUserName, strSMSBoxUserPswd, strHeader, coMSISDN.v, strText, strSec, strPin);
					UTL_LOG_N(coLog, "sms is sent with status '%d': '%s'; '%s'; '%s';", iFnRes, strHeader.c_str(), coMSISDN.v.c_str(), strText.c_str());
          try {
  					coResult << coMSISDN << strHeader << strText << strSec << strPin << iFnRes;
          } catch (otl_exception &coExept) {
            UTL_LOG_E (coLog, "code: '%d'; message: '%s'; query: '%s';", coExept.code, coExept.msg, coExept.stm_text);
            coResult.clean();
          }
				}

				/* отправляем уведомление о доставленных настройках */
        if (eProfile != m_eSimpleText && eProfile != m_eSimpleWnd) {
          switch (eSettingsType) {
          case m_eInternet:
            strText = g_pszNotiPostInet;
            break;
          case m_eMMS:
            strText = g_pszNotiPostMMS;
            break;
          case m_eInternetMMS:
            strText = g_pszNotiPostInetMMS;
            break;
          }
					iFnRes = put_sms(coLog, strSMSBoxHost, strSMSBoxSMSURL, strSMSBoxUserName, strSMSBoxUserPswd, strHeader, coMSISDN.v, strText);
					UTL_LOG_N(coLog, "sms is sent with status '%d': '%s'; '%s'; '%s';", iFnRes, strHeader.c_str(), coMSISDN.v.c_str(), strText.c_str());
          try {
            coResult << coMSISDN << strHeader << strText << strSec << strPin << iFnRes;
          } catch (otl_exception &coExept) {
            UTL_LOG_E (coLog, "code: '%d'; message: '%s'; query: '%s';", coExept.code, coExept.msg, coExept.stm_text);
            coResult.clean();
          }
        }
        delete_and_continue:
				coDelete << coRowId;
			}
		} catch (otl_exception &coExept) {
			UTL_LOG_E(coLog, "code: '%d'; message: '%s'; query: '%s';", coExept.code, coExept.msg, coExept.stm_text);
		}
	} else {
		UTL_LOG_E(coLog, "can't get DB connection");
	}

	if (pcoDBConn) {
		db_pool_release(pcoDBConn);
	}

	/* очищаем CURL */
	curl_global_cleanup();
	/* освобождаем пул подключений к БД */
	db_pool_deinit();

	return iRetVal;
}

int put_sms(
	CLog &p_coLog,
	std::string p_strHost,
	std::string p_strURL,
	std::string p_strUserName,
	std::string p_strUserPswd,
	std::string p_strFrom,
	std::string p_strTo,
	std::string p_strText)
{
	int iRetVal = 0;
	int iFnRes;

	/* инициализация дескриптора */
	CURL *pCurl = curl_easy_init();
	if (NULL == pCurl) {
		return ENOMEM;
	}

	char mcError[CURL_ERROR_SIZE];
	curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, mcError);

	/* проверяем кодировку строки */
	std::string strText;
	std::string strCoding;
	if (!is_ascii_string(p_strText.data(), p_strText.length())) {
		iFnRes = conv_string_to_ucs2(p_coLog, p_strText, strText);
		strCoding = '2';
	} else {
		strText = p_strText;
		strCoding = '0';
	}

	do {
		CURLcode curlCode;
		curl_slist *psoList = NULL;
		std::string strHeader;
		/* формируем необходимые HTTP-заголовки */
		/* добавляем в заголовки Host */
		strHeader = "Host: " + p_strHost;
		psoList = curl_slist_append(psoList, strHeader.c_str());
		/* добавляем в заголовки Connection */
		strHeader = "Connection: close";
		psoList = curl_slist_append(psoList, strHeader.c_str());
		/* добавляем список заголовков в HTTP-запрос */
		curlCode = curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, psoList);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_HTTPHEADER", curlCode, mcError);
			iRetVal = -100;
			break;
		}
		/* задаем User-Agent */
		curlCode = curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "smsboxclient/0.1");
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_USERAGENT", curlCode, mcError);
			iRetVal = -101;
			break;
		}
		/* формируем URL */
		bool bFirst = true;
		std::string strURL;
		std::string strParam;
		strURL += p_strHost;
		strURL += p_strURL;
		append_urlparam(pCurl, strParam, "username", p_strUserName, true, bFirst);
		append_urlparam(pCurl, strParam, "password", p_strUserPswd, true, bFirst);
		append_urlparam(pCurl, strParam, "from", p_strFrom, true, bFirst);
		append_urlparam(pCurl, strParam, "to", p_strTo, true, bFirst);
		append_urlparam(pCurl, strParam, "text", strText, true, bFirst);
		append_urlparam(pCurl, strParam, "coding", strCoding, true, bFirst);
		strURL += strParam;
		curlCode = curl_easy_setopt(pCurl, CURLOPT_URL, strURL.c_str());
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_URL", curlCode, mcError);
			iRetVal = -103;
			break;
		}
		/* задаем тип запроса GET */
		curlCode = curl_easy_setopt(pCurl, CURLOPT_HTTPGET, 1L);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_HTTPGET", curlCode, mcError);
			iRetVal = -104;
			break;
		}
		UTL_LOG_N(p_coLog, "try to send request: '%s';", strURL.c_str());
		curlCode = curl_easy_perform(pCurl);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't execute request: error code: '%d'; description: '%s'; url: '%s';", curlCode, mcError, strURL.c_str());
			iRetVal = -105;
			break;
		}
		long lResulCode;
		curlCode = curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &lResulCode);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't retrieve request result code: error code: '%d'; description: '%s';", curlCode, mcError);
			iRetVal = -106;
			break;
		}
		UTL_LOG_N(p_coLog, "request is sent successfully: response code: '%u';", lResulCode);
		/* операция завершена успешно */
		iRetVal = lResulCode;
	} while (0);

	/* очистка дескриптора */
	if (pCurl) {
		curl_easy_cleanup(pCurl);
	}

	return iRetVal;
}

int is_ascii_string(
	const char *p_pszString,
	size_t p_stStrLen)
{
	int iRetVal = 1;

	for (size_t i = 0; i < p_stStrLen; ++i) {
		if ((unsigned int)p_pszString[i] > 127) {
			iRetVal = 0;
			break;
		}
	}

	return iRetVal;
}

int conv_string_to_ucs2(
	CLog &p_coLog,
	std::string &p_strString,
	std::string &p_strOut)
{
	int iRetVal = 0;

	iconv_t tIConv;

	tIConv = iconv_open("UCS-2BE", "UTF-8");
	if (tIConv == (iconv_t)-1) {
		iRetVal = errno;
		UTL_LOG_E(p_coLog, "can't create iconv descriptor; error: code: '%d';", iRetVal);
		return iRetVal;
	}

	size_t stStrLen;
	const char *pszInBuf;
	char *pszResult;
	size_t stResultSize;
	char *pszOut;

	pszInBuf = p_strString.data();
	stStrLen = p_strString.length();
	stResultSize = stStrLen * 4;

	pszResult = (char*)malloc(stResultSize);
	pszOut = pszResult;
	stStrLen = iconv(tIConv, (char**)(&pszInBuf), &stStrLen, &pszOut, &stResultSize);
	if (stStrLen == (size_t)-1) {
		iRetVal = errno;
		UTL_LOG_E(p_coLog, "iconv conversion error: code: '%d';", iRetVal);
		goto clean_and_exit;
	}

	stStrLen = pszOut - pszResult;

	p_strOut.insert(0, pszResult, stStrLen);

clean_and_exit:

	iconv_close(tIConv);

	if (pszResult) {
		free(pszResult);
	}

	return iRetVal;
}

int put_ota(
	CLog &p_coLog,
	std::string &p_strHost,
	std::string &p_strURL,
	std::string &p_strUserName,
	std::string &p_strUserPswd,
	std::string &p_strFrom,
	std::string &p_strTo,
	std::string &p_strText,
	std::string &p_strSec,
	std::string &p_strPin)
{
	int iRetVal = 0;

	/* инициализация дескриптора */
	CURL *pCurl = curl_easy_init();
	if (NULL == pCurl) {
		return ENOMEM;
	}

	char mcError[CURL_ERROR_SIZE];
	curl_easy_setopt(pCurl, CURLOPT_ERRORBUFFER, mcError);

	do {
		CURLcode curlCode;
		curl_slist *psoList = NULL;
		std::string strHeader;
		/* формируем необходимые HTTP-заголовки */
		/* добавляем в заголовки Host */
		strHeader = "Host: " + p_strHost;
		psoList = curl_slist_append(psoList, strHeader.c_str());
		/* добавляем в заголовки Connection */
		strHeader = "Connection: close";
		psoList = curl_slist_append(psoList, strHeader.c_str());
		/* добавляем список заголовков в HTTP-запрос */
		curlCode = curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, psoList);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_HTTPHEADER", curlCode, mcError);
			iRetVal = -100;
			break;
		}
		/* задаем User-Agent */
		curlCode = curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "omaotaclient/0.1");
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_USERAGENT", curlCode, mcError);
			iRetVal = -101;
			break;
		}
		/* формируем URL */
		bool bFirst = true;
		std::string strURL;
		std::string strParam;
		strURL += p_strHost;
		strURL += p_strURL;
		append_urlparam(pCurl, strParam, "username", p_strUserName, true, bFirst);
		append_urlparam(pCurl, strParam, "password", p_strUserPswd, true, bFirst);
		if (p_strFrom.length())
			append_urlparam(pCurl, strParam, "from", p_strFrom, true, bFirst);
		append_urlparam(pCurl, strParam, "to", p_strTo, true, bFirst);
		append_urlparam(pCurl, strParam, "text", p_strText, true, bFirst);
		std::string strType("oma-settings");
		append_urlparam(pCurl, strParam, "type", strType, true, bFirst);
		if (p_strSec.length())
			append_urlparam(pCurl, strParam, "sec", p_strSec, true, bFirst);
		if (p_strPin.length())
			append_urlparam(pCurl, strParam, "pin", p_strPin, false, bFirst);
		strURL += strParam;
		curlCode = curl_easy_setopt(pCurl, CURLOPT_URL, strURL.c_str());
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_URL", curlCode, mcError);
			iRetVal = -103;
			break;
		}
		/* задаем тип запроса GET */
		curlCode = curl_easy_setopt(pCurl, CURLOPT_HTTPGET, 1L);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't set option '%s'; error: code: '%d'; description: '%s';", "CURLOPT_HTTPGET", curlCode, mcError);
			iRetVal = -104;
			break;
		}
		UTL_LOG_N(p_coLog, "try to send request: '%s';", strURL.c_str());
		curlCode = curl_easy_perform(pCurl);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't execute request: error code: '%d'; description: '%s'; url: '%s';", curlCode, mcError, strURL.c_str());
			iRetVal = -105;
			break;
		}
		long lResulCode;
		curlCode = curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &lResulCode);
		if (curlCode) {
			UTL_LOG_E(p_coLog, "can't retrieve request result code: error code: '%d'; description: '%s';", curlCode, mcError);
			iRetVal = -106;
			break;
		}
		UTL_LOG_N(p_coLog, "request is sent successfully: response code: '%u';", lResulCode);
		/* операция завершена успешно */
		iRetVal = lResulCode;
	} while (0);

	/* очистка дескриптора */
	if (pCurl) {
		curl_easy_cleanup(pCurl);
	}

	return iRetVal;
}

void append_urlparam(
	CURL *pCurl,
	std::string &p_strParamSet,
	const char *p_pszParamName,
	std::string &p_strParamVal,
  bool p_bValueURLEncode,
	bool &p_bFirstParam)
{
	char *pszEncodedString;
	if (p_bFirstParam) {
		p_strParamSet += '?';
		p_bFirstParam = false;
	} else {
		p_strParamSet += '&';
	}
	if (p_pszParamName) {
		pszEncodedString = curl_easy_escape(pCurl, p_pszParamName, 0);
		if (pszEncodedString) {
			p_strParamSet += pszEncodedString;
			p_strParamSet += '=';
			curl_free(pszEncodedString);
		}
	}
	if (p_strParamVal.length()) {
    if ( p_bValueURLEncode ) {
      pszEncodedString = curl_easy_escape( pCurl, p_strParamVal.data(), p_strParamVal.length() );
      if ( pszEncodedString ) {
        p_strParamSet += pszEncodedString;
        curl_free( pszEncodedString );
      }
    } else {
      p_strParamSet += p_strParamVal;
    }
	}
}

void IMSI2SemiOctet(
	std::string &p_strIMSI,
	std::string &p_strSemiOctet)
{
  if ( p_strIMSI.length() == 0 ) {
    return;
  }

  p_strSemiOctet = '%';
	p_strSemiOctet += p_strIMSI[0];

  if( 0 != p_strIMSI.length() % 2 ) {
    p_strSemiOctet += '9';
  } else {
    p_strSemiOctet += '8';
  }

	for (size_t i = 1; i < p_strIMSI.length(); ++i, ++i) {
    p_strSemiOctet += '%';
    if ( i + 1 < p_strIMSI.length() ) {
      p_strSemiOctet += p_strIMSI[ i + 1 ];
    } else {
      p_strSemiOctet += 'F';
    }
		p_strSemiOctet += p_strIMSI[i];
	}
}
