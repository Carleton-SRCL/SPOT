import os
import tensorflow as tf
import csv

trained_checkpoint_prefix = 'Iteration_2525876.ckpt'
export_dir = os.path.join('export_dir', '0')

graph = tf.Graph()
with tf.compat.v1.Session(graph=graph) as sess:
    # Restore from checkpoint
    loader = tf.compat.v1.train.import_meta_graph(trained_checkpoint_prefix + '.meta')
    loader.restore(sess, trained_checkpoint_prefix)
    
    # Get the list of operations in the graph
    operations = graph.get_operations()

    # Print the names of the tensors in the graph
    #for operation in operations:
        #print(operation.name)
    
    weights = graph.get_tensor_by_name("agent_10/output_layer/kernel:0")
    biases = graph.get_tensor_by_name("agent_10/output_layer/bias:0")

    # Export checkpoint to SavedModel
    #builder = tf.compat.v1.saved_model.builder.SavedModelBuilder(export_dir)
    #builder.add_meta_graph_and_variables(sess,
    #                                     [tf.saved_model.TRAINING, tf.saved_model.SERVING],
    #                                     strip_default_attrs=True)
    
    weights_val, biases_val = sess.run([weights, biases])
    print(weights)
    print(biases)
    #builder.save() 
    #print(builder)
    
# Write the values to a CSV file
with open("weights_output.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerows(weights_val)

with open("biases_output.csv", "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(biases_val)
    
