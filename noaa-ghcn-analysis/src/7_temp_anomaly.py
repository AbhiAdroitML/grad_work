#!/usr/bin/env python
# coding: utf-8

# Connect to BigQuery
from google.cloud import bigquery
from pyspark.sql import SparkSession
from pyspark.sql.functions import mean
from pyspark.sql.functions import avg, lag, when, col
from pyspark.sql.window import Window
import pandas as pd

# Query the data from BigQuery
project_id = 'abbajpai-weather-pipeline'
dataset_id = 'noaa_data_analysis'
table_id = 'ghcnd_avg_yearly_temp'

# Run a query
query = f'SELECT * FROM {project_id}.{dataset_id}.{table_id} WHERE temp_record_year < 2023'
df = pd.read_gbq(query=query, project_id=project_id, dialect='standard')

spark = SparkSession.builder.appName('Temperature_Anomaly').getOrCreate()
avg_temp_df= spark.createDataFrame(df)

# Compute the reference temperature as the average temperature for the base period
ref_temp_df = avg_temp_df.filter(avg_temp_df.temp_record_year.between(1973, 1983)).groupBy('country_name', 'element')                         .agg(avg('mean_temp').alias('ref_temp'))

# Compute the temperature anomaly as the difference between the average temperature and the reference temperature
anomaly_df = avg_temp_df.join(ref_temp_df, on=['country_name', 'element']).withColumn('anomaly', col('mean_temp') - col('ref_temp'))

# Writing the results to GCS Storage
anomaly_df.write.format("csv").option("header", True).mode("overwrite").save("gs://abbajpai-noaa/anomaly-1973-2023")

# Filter the anomaly data for the United States
# import matplotlib.pyplot as plt

# usa_anomaly_df = anomaly_df.filter(anomaly_df.country_name == 'United States')

# # Convert the anomaly DataFrame to a Pandas DataFrame for plotting
# usa_anomaly_pd = usa_anomaly_df.select('temp_record_year', 'element', 'anomaly').toPandas()

# # Plot the temperature anomaly for the United States
# fig, ax = plt.subplots()

# for element in usa_anomaly_pd.element.unique():
#     element_pd = usa_anomaly_pd[usa_anomaly_pd.element == element]
#     ax.plot(element_pd.temp_record_year, element_pd.anomaly, label=element)

# ax.set_xlabel('Year')
# ax.set_ylabel('Temperature anomaly (°C)')
# ax.set_title('Temperature anomaly for the United States')
# ax.legend()
# plt.show()

