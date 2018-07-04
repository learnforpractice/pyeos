import os
import time
import gzip
import shutil
import logging
import tarfile
import hashlib
import traceback

import urllib.request as urllib2
import urllib.parse as urlparse
from multiprocessing import Process
from logging.config import dictConfig

logging_config = dict(
    version = 1,
    formatters = {
        'f': {'format':
              '%(asctime)s %(name)-12s %(module)-12s %(lineno)-4d %(levelname)-8s %(message)s'}
        },
    handlers = {
        'h1': {'class': 'logging.StreamHandler',
              'formatter': 'f',
              'level': logging.DEBUG},
        'h2': {'class': 'logging.FileHandler',
              'formatter': 'f',
              'level': logging.DEBUG,
              'filename':'log {0}{1}'.format(time.strftime('%y-%m-%d %X'),'.txt')}
        },
    root = {
        'handlers': ['h1', 'h2'],
        'level': logging.DEBUG,
        },
)

dictConfig(logging_config)
logger = logging.getLogger()

class Upgrade(object):
    def __init__(self, version, url, hash_code):
        self.url = url
        self.version = version
        self.hash_code = hash_code
        if not os.path.exists('upgrade'):
            os.mkdir('upgrade')
        self.upgrade_dir = 'upgrade/version.{0}'.format(version)
        if not os.path.exists(self.upgrade_dir):
            os.mkdir(self.upgrade_dir)

    def start(self):
        logger.info('upgrading start...')
        try:
            upgrade_file = os.path.join(self.upgrade_dir, 'upgrade.{0}.tar.gz'.format(self.version))
            if not os.path.exists(upgrade_file):
                upgrade_file = self.download()
            if not self.verify(upgrade_file):
                return False
            if not self.extract_file(upgrade_file):
                return False
        except:
            traceback.print_exc()

    #reference from https://stackoverflow.com/questions/22676/how-do-i-download-a-file-over-http-using-python
    def download(self):
        """ 
        Download and save a file specified by url to dest directory,
        """
        logger.info("downlaoding upgrade file from {0}".format(self.url))
        
        u = urllib2.urlopen(self.url)
        logger.info("go here.")
        scheme, netloc, path, query, fragment = urlparse.urlsplit(self.url)
        filename = os.path.basename(path)
        if not filename:
            filename = 'update.file'
        filename = os.path.join(self.upgrade_dir, filename)
        logger.info("go here.")
        with open(filename, 'wb') as f:
            meta = u.info()
            meta_func = meta.getheaders if hasattr(meta, 'getheaders') else meta.get_all
            meta_length = meta_func("Content-Length")
            file_size = None
            if meta_length:
                file_size = int(meta_length[0])
            logger.info("Downloading: {0} Bytes: {1}".format(self.url, file_size))
    
            file_size_dl = 0
            block_sz = 8192
            while True:
                buffer = u.read(block_sz)
                if not buffer:
                    break
    
                file_size_dl += len(buffer)
                f.write(buffer)
    
                status = "{0:16}".format(file_size_dl)
                if file_size:
                    status += "   [{0:6.2f}%]".format(file_size_dl * 100 / file_size)
                status += chr(13)
                print(status, end="")
            print()
        return filename

    def verify(self, upgrade_file):
        try:
            m = hashlib.sha256()
            f = open(upgrade_file, 'rb')
            while True:
                a = f.read(1024*1024)
                if not a:
                    break
                m.update(a)
            checksum = m.hexdigest()
            if checksum == self.hash_code:
                logger.info('upgrade file verify passed')
                return True
            else:
                logger.info('checksum is wrong, \nshould be {0}, \nactually {1}'.format(checksum, self.hash_code))
                return False
        except:
            traceback.print_exc()
        return False

    def extract_file(self, tar_file):
        try:
            tar = tarfile.open(tar_file)
            tar.extractall(self.upgrade_dir)
            tar.close()
        except:
            logger.exception('')

download_url = 'http://127.0.0.1:9088/eosio.tar.gz'
verify_code = 'd4c6ebe1f42cb4d57e39b9720286ef1cc5d7480a4aa63ab2bddb965da278e07b'

def f(version, url, verify_code):
    print('child process pid', os.getpid())
    try:
        up = Upgrade(version, url, verify_code)
        up.start()
    except:
        logger.exception('')

def upgrade(version):
    print('parent process pid', os.getpid())
    p = Process(target=f, args=(version, download_url, verify_code))
    p.start()
    p.join()
