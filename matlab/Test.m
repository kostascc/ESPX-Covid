format longg;

%BTnear = table2array(readtable('../BTnear.txt'));
Upload = table2array(readtable('../upload.txt'));
%CovidTest = table2array(readtable('../CovidTest.txt'));
TickLatency = table2array(readtable('../TickLatency.txt'));

figure
histogram((TickLatency(TickLatency(:,3)<9.000400, 3))*1000000, 90)
title('Histogram of BTnearMe Call Latency')
xlabel('\mus') 
ylabel('Count') 

%figure
%UploadHistory = zeros(max(Upload(:,1)), 1);
%for i=1:size(Upload, 1)
%    UploadHistory(Upload(i,1),1) = UploadHistory(Upload(i,1),1) + 1;
%end
%figure
%UploadHistory = zeros(max(Upload(:,1)), 1);
%for i=1:size(Upload, 1)
%    UploadHistory(Upload(i,1),1) = UploadHistory(Upload(i,1),1) + 1;
%end
%histogram(UploadHistory, 90)
%title('Histogram of Near-Contact Uploads')
%xlabel('Contacts') 
%ylabel('Count') 



figure
UploadHistory = zeros(max(Upload(:,1)), 1);
for i=1:max(Upload(:,1))
    a = Upload(Upload(:,1)==i, :);
    UploadHistory(i,1) = size(unique(a(:,3),'rows'), 1);
    %UploadHistory(i,1) = size(a(:,1), 1);
end
histogram(UploadHistory, 90)
title('Near-Contact Uploads History')
xlabel('Contacts') 
ylabel('Count') 



figure
%UploadHistoryMean = movmean(UploadHistory,50);
plot(1:size(UploadHistory, 1), UploadHistory);
hold on;
%plot(1:size(UploadHistory, 1), UploadHistoryMean);
title('Near-Contact Uploads History')
xlabel('Upload after Positive Test') 
ylabel('Near Cοntacts') 
