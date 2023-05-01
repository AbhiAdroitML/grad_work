from airflow import DAG
from airflow.contrib.operators.dataproc_operator import DataprocClusterCreateOperator
from airflow.contrib.operators.dataproc_operator import DataProcPySparkOperator
from airflow.contrib.operators.dataproc_operator import DataprocClusterDeleteOperator
from datetime import datetime, timedelta

default_args = {
    'owner': 'airflow',
    'depends_on_past': False,
    'start_date': datetime(2023, 4, 15),
    'email_on_failure': False,
    'email_on_retry': False,
    'retries': 1,
    'retry_delay': timedelta(minutes=5),
}

dag = DAG('dataproc_bigquery_noaa',
          default_args=default_args,
          schedule_interval=timedelta(days=1),
          catchup=False)

create_cluster = DataprocClusterCreateOperator(
    task_id='create_dataproc_cluster',
    cluster_name='dataproc-cluster-ab-noaa',
    project_id='abbajpai-weather-pipeline',
    num_workers=2,
    init_actions_uris=['gs://dataproc-initialization-actions/connectors/connectors.sh'],
    optional_components=["ANACONDA"],
    master_machine_type='n1-standard-4',
    worker_machine_type='n1-standard-4',
    region='us-central1',
    service_account='XXXXXXXXX-compute@developer.gserviceaccount.com', # Add actual service account details
    storage_bucket='abbajpai-noaa',
    dag=dag
)

run_spark_job = DataProcPySparkOperator(
    task_id='run_spark_job',
    main='gs://abbajpai-noaa/my_script.py',
    cluster_name='dataproc-cluster-ab-noaa',
    arguments=['--input', 'noaa_data_analysis.ghcnd_all_temp_1973_2023', '--output', 'noaa_data_analysis.ghcnd_anomaly_1973_2023'],
    region='us-central1',
    dag=dag
)

delete_cluster = DataprocClusterDeleteOperator(
    task_id='delete_dataproc_cluster',
    cluster_name='dataproc-cluster-ab-noaa',
    project_id='abbajpai-weather-pipeline',
    region='us-central1',
    dag=dag
)

create_cluster >> run_spark_job >> delete_cluster
