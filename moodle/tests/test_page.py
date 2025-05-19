import time

import chromedriver_autoinstaller
import pytest
from selenium import webdriver
from selenium.webdriver.common.by import By

NO_PARAMS_URL = 'http://127.0.0.1:5000'
PARAMS_URL = NO_PARAMS_URL + '/?session_id=c63535ee'


@pytest.fixture()
def chrome():
    chromedriver_autoinstaller.install()

    chrome_options = webdriver.ChromeOptions()
    chrome_options.add_argument("--headless")

    driver = webdriver.Chrome(options=chrome_options)

    driver.implicitly_wait(10)
    yield driver

    driver.quit()


def test_open_page_no_params(chrome):
    chrome.get(NO_PARAMS_URL)
    assert chrome.title == 'Ripes'


def test_open_page_with_params(chrome):
    chrome.get(PARAMS_URL)
    assert chrome.title == 'Ripes'




