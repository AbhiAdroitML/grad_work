from flask import Flask, request, url_for, redirect, render_template, jsonify
import pandas as pd
import pickle
import os
import scipy
import sys

from sentence_transformers import SentenceTransformer # Import your semantic search module
model = SentenceTransformer('sentence-transformers/msmarco-distilbert-base-dot-prod-v3')

df = pd.read_pickle('./Models/duchas_03_30_2023.pkl')
sentences = df['clean_sentence'].values.tolist()
filename = os.path.join("./Models","duchas_sentence_embeddings_0330.pkl")
unpickleFile = open(filename, 'rb')
sentence_embeddings = pickle.load(unpickleFile, encoding='bytes')

app = Flask(__name__)

@app.route('/')
def home():
    return render_template("index.html")

@app.route('/search_api',methods=['POST'])
def search_api():
    query_list = list([x for x in request.form.values()])
    print(query_list)
    output_results = []
    df['search_query']=None
    df['relevance_score']=None
    #query_list = []
    #query_list.append(query)
    # Single search or multiple search
    # query_list = ["reporting is bad"]
    # query_list = ["fear of tinkers"]
    for query in query_list:
        queries = [query]
        query_embeddings = model.encode(queries)
        index_list =[]
        # Find the closest sentences of the corpus for each query sentence based on cosine similarity
        number_top_matches = 50

        #print("Semantic Search Results")

        for query, query_embedding in zip(queries, query_embeddings):
            distances = scipy.spatial.distance.cdist([query_embedding], sentence_embeddings, "cosine")[0]

            results = zip(range(len(distances)), distances)
            results = sorted(results, key=lambda x: x[1])

            #print("\n\n======================\n\n")
            #print("Query:", query)
            # print("\n Most similar sentences in corpus:")

            for idx, distance in results[0:number_top_matches]:
                print(sentences[idx].strip(), "(Cosine Score: %.4f)" % (1-distance))
                index_list.append(idx)
                df.loc[idx,'relevance_score'] = (1-distance)*100
                df.loc[idx, 'search_query'] = query

        output_results.append(df.loc[df.index[index_list]])

    query_results = pd.concat(output_results)
    first_column = query_results.pop('search_query')
    third_column = query_results.pop('relevance_score')
    query_results.insert(0, 'search_query', first_column)
    query_results.insert(3, 'relevance_score', third_column)
    query_results.head()
    table_html = query_results[['search_query','volume', 'volumenumber','pagenumber','relevance_score','clean_sentence']].to_html(index=False, header=True, classes='table')
    return render_template('results.html', table_html=table_html)

if __name__ == '__main__':
    app.run(debug=True)
#%%

