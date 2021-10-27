cd python_api
rm -f VnVReader.py
rm -f _VnVReader.so
ln -s $1/report-generation/python_api/VnVReader.py
ln -s $1/report-generation/python_api/_VnVReader.so
